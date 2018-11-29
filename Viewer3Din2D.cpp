#include "stdafx.h"

#include <assert.h>

#include <osg/LineWidth>
#include <osgGA/TrackballManipulator>

#include "ViewportFrame.h"
#include "ZoomPanManipulator.h"

#include "Viewer3Din2D.h"

class IndepentSlaveCallback : public osg::View::Slave::UpdateSlaveCallback
{
    osg::ref_ptr<osgGA::CameraManipulator> m_camMan;
public:
    IndepentSlaveCallback(osgGA::CameraManipulator* camMan)
        : m_camMan(camMan)
    {
    }
    virtual void updateSlave(osg::View& view, osg::View::Slave& slave) override
    {
        m_camMan->updateCamera(*slave._camera.get());
    }
    osg::ref_ptr<osgGA::CameraManipulator>& getCameraManipulator()
    {
        return m_camMan;
    }
};

inline bool InRect(double x, double y, double width, double height, double px, double py)
{
    return px >= x && px <= x + width && py >= y && py <= y + height;
}

class MasterCameraHandler : public osgGA::GUIEventHandler
{
public:
    MasterCameraHandler(Viewer3Din2D* view)
        : m_view(view)
    {

    }
    ~MasterCameraHandler()
    {

    }
    //META_Object(osgGA, ZoomPanManipulator)
    virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa) override
    {
        if (m_view->m_activeManipulatorIndex == -1)
        {
            switch (ea.getEventType())
            {
            case osgGA::GUIEventAdapter::DOUBLECLICK:
                if (ea.getButton() == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
                {
                    int i = m_view->ViewportHit(ea.getX(), ea.getY());
                    if (m_view->ActivateCameraManipulator(i, true))
                    {
                        return true;
                    }
                }
                break;
            }
        }
        else
        {
            switch (ea.getEventType())
            {
            case osgGA::GUIEventAdapter::PUSH:
                if (ea.getButton() == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
                {
                    int i = m_view->ViewportHit(ea.getX(), ea.getY());
                    if (m_view->ActivateCameraManipulator(i, false))
                    {
                        return true;
                    }
                }
                break;
            }

        }
        switch (ea.getEventType())
        {
        case osgGA::GUIEventAdapter::RESIZE:
            m_view->UpdateViewportFrames();
            m_view->m_masterCameraManipulator->ZoomAll(m_view);
            break;
        }
        return false;
    }

    Viewer3Din2D* m_view;
};

Viewer3Din2D::Viewer3Din2D()
    : m_activeManipulatorIndex(-1)
{
    auto master = getCamera();
    // prevent OSG to generate CameraManipulator for master camera.
    master->setAllowEventFocus(false);
    m_masterCameraHandler = new MasterCameraHandler(this);
    m_masterCameraManipulator = new ZoomPanManipulator;
    master->addEventCallback(m_masterCameraHandler.get());
    master->addEventCallback(m_masterCameraManipulator.get());
}


Viewer3Din2D::~Viewer3Din2D()
{
}

bool Viewer3Din2D::addSlave(osg::Camera * camera, osg::Group * sceneGraph, osgGA::CameraManipulator * cameraManipulator)
{
    assert(camera);
    assert(sceneGraph);
    bool ret = base::addSlave(camera, false);
    if (ret)
    {
        camera->getOrCreateStateSet()->setMode(GL_SCISSOR_TEST, GL_TRUE);
        // slave cameras must keep same scalability on X and Y axis, so viewport must coorperate with projection.
        // getCamera()->setProjectionResizePolicy(osg::Camera::ProjectionResizePolicy::HORIZONTAL);
        // see GraphicsContext::resizedImplementation
        camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
        camera->addChild(sceneGraph);
        //camera->setClearMask(0);
        //camera->setAllowEventFocus(false);
        auto i = getNumSlaves();
        auto& slave = getSlave(i - 1);
        if (!cameraManipulator)
            cameraManipulator = new osgGA::TrackballManipulator;
        slave._updateSlaveCallback = new IndepentSlaveCallback(cameraManipulator);
        m_cameraManipulators.push_back(cameraManipulator);
        cameraManipulator->setNode(sceneGraph);
        osg::ref_ptr<osgGA::GUIEventAdapter> dummyEvent = getEventQueue()->createEvent();
        cameraManipulator->home(*dummyEvent, *this);
    }
    return ret;
}

bool Viewer3Din2D::ActivateCameraManipulator(int i, bool activate)
{
    assert(i >= -1 && i < int(getNumSlaves()));
    if (m_activeManipulatorIndex == i)
        return false;
    EnableCameraManipulator(m_activeManipulatorIndex, &osg::Node::removeEventCallback, 1.f);
    if (activate)
    {
        m_activeManipulatorIndex = i;
    }
    else
    {
        m_activeManipulatorIndex = -1;
    }
    EnableCameraManipulator(m_activeManipulatorIndex, &osg::Node::addEventCallback, 3.f);
    return true;
}

void Viewer3Din2D::realize()
{
    base::realize();
    auto gc = getCamera()->getGraphicsContext();
    assert(gc);
    auto num = getNumSlaves();
    for (decltype(num) i = 0; i < num; ++i)
    {
        auto& slave = getSlave(i);
        slave._camera->setGraphicsContext(gc);
        auto* uscb = dynamic_cast<IndepentSlaveCallback*>(slave._updateSlaveCallback.get());
        if (uscb)
        {
            auto man = uscb->getCameraManipulator();
        }
    }
}

void Viewer3Din2D::EnableCameraManipulator(int i, ChangeEventCallback changeEventCallback, float linewidth)
{
    if (i == -1)
    {
        auto master = getCamera();
        (master->*changeEventCallback)(m_masterCameraManipulator.get());
    }
    else
    {
        auto& slave = getSlave(i);
        auto* uscb = dynamic_cast<IndepentSlaveCallback*>(slave._updateSlaveCallback.get());
        if (uscb)
        {
            auto man = uscb->getCameraManipulator();
            if (man.valid())
            {
                (slave._camera->*changeEventCallback)(man.get());
            }
        }
        auto ss = m_viewportFrames[i]->getStateSet();
        assert(ss);
        auto lw = ss->getAttribute(osg::StateAttribute::LINEWIDTH);
        static_cast<osg::LineWidth*>(lw)->setWidth(linewidth);
    }
}

int Viewer3Din2D::ViewportHit(double x, double y)
{
    auto num = getNumSlaves();
    for (decltype(num) i = 0; i < num; ++i)
    {
        auto& slave = getSlave(i);
        auto vp = slave._camera->getViewport();
        if (InRect(vp->x(), vp->y(), vp->width(), vp->height(), x, y))
        {
            return i;
        }
    }
    return -1;
}

void Viewer3Din2D::UpdateViewportFrames(/*ZoomPanManipulator* zoom*/)
{
    auto root = getSceneData();
    auto group = root->asGroup();
    assert(group);
    auto transform = group->asTransform();
    assert(!transform);

    m_viewportDims.clear();
    auto cnt = group->getNumChildren();
    for (int i = int(cnt) - 1; i >= 0; --i)
    {
        auto child = group->getChild(i);
        auto frame = dynamic_cast<ViewportFrame*>(child);
        if (frame)
        {
            group->removeChildren(i, 1);
        }
    }
    m_viewportFrames.clear();

    auto num = getNumSlaves();
    m_viewportDims.reserve(num);
    for (decltype(num) i = 0; i < num; ++i)
    {
        auto& slave = getSlave(i);
        auto vp = slave._camera->getViewport();
        osg::ref_ptr<ViewportFrame> frame = new ViewportFrame;
        m_viewportFrames.push_back(frame.get());
        frame->setRect(slave._camera.get());
        group->addChild(frame.get());
        m_viewportDims.push_back({ vp->x(), vp->y(), vp->width(), vp->height() });
    }
}

void Viewer3Din2D::UpdateViewport(double l, double b, double zoom)
{
    auto master_vp = getCamera()->getViewport();
    auto num = getNumSlaves();
    assert(num == unsigned int(m_viewportDims.size()));
    for (decltype(num) i = 0; i < num; ++i)
    {
        auto& slave = getSlave(i);
        double z_1 = 1 / zoom;
        auto vp = slave._camera->getViewport();
        vp->x() = (m_viewportDims[i].x - l) * z_1 + master_vp->x();
        vp->y() = (m_viewportDims[i].y - b) * z_1 + master_vp->y();
        vp->width() = m_viewportDims[i].width * z_1;
        vp->height() = m_viewportDims[i].height * z_1;
    }
}

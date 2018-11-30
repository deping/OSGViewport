#include "stdafx.h"

#include <assert.h>

#include <osg/LineWidth>
#include <osgGA/TrackballManipulator>

#include "ViewportFrame.h"
#include "ZoomPanManipulator.h"

#include "Viewer3Din2D.h"

class IndepentSlaveCallback : public osg::View::Slave::UpdateSlaveCallback
{
    osg::ref_ptr<osgGA::GUIEventHandler> m_camMan;
public:
    IndepentSlaveCallback(osgGA::GUIEventHandler* camMan)
        : m_camMan(camMan)
    {
    }
    // see Viewer::updateTraversal
    virtual void updateSlave(osg::View& view, osg::View::Slave& slave) override
    {
        auto camMan = dynamic_cast<osgGA::CameraManipulator*>(m_camMan.get());
        if (camMan)
        {
            camMan->updateCamera(*slave._camera.get());
        }
        else
        {
            // camera is already updated by ZoomPanManipulator
        }
    }
    osg::ref_ptr<osgGA::GUIEventHandler>& getCameraManipulator()
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
        , m_mode(DragMode::None)
        , m_viewportIndex(-1)
    {

    }
    ~MasterCameraHandler()
    {

    }
    enum class DragMode { None, DragViewport };
    //META_Object(osgGA, ZoomPanManipulator)
    virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa) override
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
        case osgGA::GUIEventAdapter::PUSH:
            if (ea.getButton() == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
            {
                int i = m_view->ViewportHit(ea.getX(), ea.getY());
                if (m_view->ActivateCameraManipulator(i, false))
                {
                    // Deactivate and/or activate some slave viewport.
                    return true;
                }
                else if (i >= 0 && m_view->m_activeManipulatorIndex == -1)
                {
                    // Drag viewport
                    m_mode = DragMode::DragViewport;
                    m_viewportIndex = i;
                    m_cursorX = ea.getX();
                    m_cursorY = ea.getY();
                    return true;
                }
                else
                {
                    // Drag objects in master camera.
                }
            }
            break;
        case(osgGA::GUIEventAdapter::RELEASE):
            if (ea.getButton() == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
            {
                if (m_mode == DragMode::DragViewport)
                {
                    m_mode = DragMode::None;
                    m_viewportIndex = -1;
                    return true;
                }
            }
            break;
        case(osgGA::GUIEventAdapter::DRAG):
            if (ea.getButtonMask() == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
            {
                if (m_mode == DragMode::DragViewport && m_viewportIndex >= 0)
                {
                    m_view->MoveViewport(m_viewportIndex, ea.getX() - m_cursorX, ea.getY() - m_cursorY);
                    m_cursorX = ea.getX();
                    m_cursorY = ea.getY();
                    return true;
                }
            }
            break;
            break;
        }

        return false;
    }

    Viewer3Din2D* m_view;
    DragMode m_mode;
    float m_cursorX, m_cursorY;
    int m_viewportIndex;
};

struct MyResizedCallback : public osg::GraphicsContext::ResizedCallback
{
    // Adapted from GraphicsContext::resizedImplementation
    virtual void resizedImplementation(osg::GraphicsContext* gc, int x, int y, int width, int height) override
    {
        std::set<osg::Viewport*> processedViewports;

        auto _traits = const_cast<osg::GraphicsContext::Traits*>(gc->getTraits());
        if (!_traits) return;

        double widthChangeRatio = double(width) / double(_traits->width);
        double heigtChangeRatio = double(height) / double(_traits->height);

        auto _cameras = gc->getCameras();

        for (osg::GraphicsContext::Cameras::iterator itr = _cameras.begin();
            itr != _cameras.end();
            ++itr)
        {
            osg::Camera* camera = (*itr);

            // resize doesn't affect Cameras set up with FBO's.
            if (camera->getRenderTargetImplementation() == osg::Camera::FRAME_BUFFER_OBJECT) continue;

            osg::Viewport* viewport = camera->getViewport();
            if (viewport)
            {
                // avoid processing a shared viewport twice
                if (processedViewports.count(viewport) == 0)
                {
                    processedViewports.insert(viewport);

                    // Only resize the master viewport
                    if (viewport->x() == 0 && viewport->y() == 0 &&
                        viewport->width() == _traits->width && viewport->height() == _traits->height)
                    {
                        viewport->setViewport(0, 0, width, height);
                    }
                    else
                    {
                    }
                }
            }

            osg::View* view = camera->getView();
            osg::View::Slave* slave = view ? view->findSlaveForCamera(camera) : 0;


            if (!slave)
            {
                // Sync project matrix with window size 
                // so that all ojbects in master camera and all slave viewport keep the same size.
                camera->getProjectionMatrix() *= osg::Matrix::scale(1/widthChangeRatio, 1/heigtChangeRatio, 1.0);
            }
        }

        _traits->x = x;
        _traits->y = y;
        _traits->width = width;
        _traits->height = height;
    }
};

Viewer3Din2D::Viewer3Din2D()
    : m_activeManipulatorIndex(-1)
{
    auto master = getCamera();
    // prevent OSG to generate CameraManipulator for master camera.
    master->setAllowEventFocus(false);
    m_masterCameraHandler = new MasterCameraHandler(this);
    m_masterCameraManipulator = new ZoomPanManipulator(this);
    master->addEventCallback(m_masterCameraHandler.get());
    master->addEventCallback(m_masterCameraManipulator.get());
}


Viewer3Din2D::~Viewer3Din2D()
{
}

bool Viewer3Din2D::addSlave(osg::Camera * camera, osg::Group * sceneGraph, osgGA::GUIEventHandler * cameraManipulator, osg::MatrixTransform* follower)
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
        m_followers.push_back(follower);
        auto camMan = dynamic_cast<osgGA::CameraManipulator*>(cameraManipulator);
        if (camMan)
        {
            camMan->setNode(sceneGraph);
            osg::ref_ptr<osgGA::GUIEventAdapter> dummyEvent = getEventQueue()->createEvent();
            camMan->home(*dummyEvent, *this);
        }
        else
        {
            auto zoom = dynamic_cast<ZoomPanManipulator*>(cameraManipulator);
            if (zoom)
                zoom->ZoomAll();
        }
    }
    return ret;
}

bool Viewer3Din2D::ActivateCameraManipulator(int i, bool activate)
{
    assert(i >= -1 && i < int(getNumSlaves()));
    if (m_activeManipulatorIndex == i)
        return false;
    if (m_activeManipulatorIndex == -1 && !activate)
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

    gc->setResizedCallback(new MyResizedCallback);
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
    // select from end to begining, so that latter viewport will be selected if more than one are hit.
    // Latter viewport will be rendered on top of its predecessors.
    for (int i = num - 1; i >= 0; --i)
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
    m_viewportDims.clear();
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

void Viewer3Din2D::MoveViewport(int i, double dx, double dy)
{
    auto& slave = getSlave(i);
    auto vp = slave._camera->getViewport();
    auto& vpld = m_viewportDims[i];
    vp->x() += dx;
    vp->y() += dy;
    double zoom = vpld.width / vp->width();
    double dlx = dx * zoom;
    double dly = dy * zoom;
    vpld.x += dlx;
    vpld.y += dly;
    m_viewportFrames[i]->MoveRect(dlx, dly);
    auto follower = m_followers[i];
    if (follower)
    {
        auto matrix = follower->getMatrix();
        auto m2 = osg::Matrix::translate(dlx, dly, 0);
        follower->setMatrix(matrix * m2);
    }
}

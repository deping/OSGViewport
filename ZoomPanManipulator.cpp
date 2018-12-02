#include "stdafx.h"
#include <assert.h>
#include <osg/ComputeBoundsVisitor>
#include <osgViewer/View>
#include <osg/Viewport>
#include "ZoomPanManipulator.h"

#ifdef HAS_VIEWER3DIN2D
#include "Viewer3Din2D.h"
#endif

inline osg::Camera* ZoomPanManipulator::getCamera() const {
#ifdef HAS_VIEWER3DIN2D
    return m_view ? m_view->getCamera() : m_camera;
#else
    return m_camera;
#endif
}

#ifdef HAS_VIEWER3DIN2D
ZoomPanManipulator::ZoomPanManipulator(Viewer3Din2D* view)
    : m_zoomFactor(1.0)
    , m_baseZoom(1.0)
    , m_zoomMode(ZoomCursor)
    , m_margin(5)
    , m_firstTime(true)
    , m_view(view)
    , m_viewportIndex(-1)
    , m_camera(nullptr)
    , m_scene(nullptr)
    , m_mode(DragMode::None)
{
    assert(view);
}
#endif

ZoomPanManipulator::ZoomPanManipulator(osg::Camera* camera, osg::Node* scene)
    : m_zoomFactor(1.0)
    , m_baseZoom(1.0)
    , m_zoomMode(ZoomCursor)
    , m_margin(5)
    , m_firstTime(true)
#ifdef HAS_VIEWER3DIN2D
    , m_view(nullptr)
    , m_viewportIndex(-1)
#endif
    , m_camera(camera)
    , m_scene(scene)
{
    assert(camera);
    assert(scene);
}

ZoomPanManipulator::~ZoomPanManipulator()
{
}

bool ZoomPanManipulator::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
#ifdef HAS_VIEWER3DIN2D
    // Don't disturb camera manipulator if it exists.
    if (m_view && m_view->getCameraManipulator())
        return false;
#endif
    auto camera = getCamera();
    if (!camera)
        return false;
    switch (ea.getEventType())
    {
    case (osgGA::GUIEventAdapter::FRAME):
        if (m_firstTime)
        {
            m_firstTime = false;
#ifdef HAS_VIEWER3DIN2D
            if (m_view)
            {
                m_view->initViewportFrames();
            }
#endif
            camera->setViewMatrixAsLookAt(osg::Vec3d(0, 0, 1), osg::Vec3d(0, 0, 0), osg::Vec3d(0, 1, 0));
            zoomAll();
            // Let other handler continue to handle this event.
            return false;
        }
        break;
    case osgGA::GUIEventAdapter::DOUBLECLICK:
        if (ea.getButton() == osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON)
        {
            zoomAll();
            return true;
        }
        break;
    case(osgGA::GUIEventAdapter::PUSH):
        if (ea.getButton() == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
        {
#ifdef HAS_VIEWER3DIN2D
            if (m_view)
            {
                int i = m_view->viewportHit(ea.getX(), ea.getY());
                if (m_view->activateCameraManipulator(i, false))
                {
                    // Deactivate some slave viewport.
                    return true;
                }
                else if (i >= 0 && m_view->m_activeManipulatorIndex == -1)
                {
                    // Drag viewport
                    m_mode = DragMode::DragViewport;
                    m_viewportIndex = i;
                    m_cursorLastX = ea.getX();
                    m_cursorLastY = ea.getY();
                    return true;
                }
                else
                {
                    // Drag objects in master camera.
                }
            }
#endif
        }
        else if (ea.getButton() == osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON)
        {
            m_mode = DragMode::Pan;
            m_cursorLastX = ea.getX();
            m_cursorLastY = ea.getY();
            return true;
        }
        break;
    case(osgGA::GUIEventAdapter::RELEASE):
        if (ea.getButton() == osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON)
        {
            m_mode = DragMode::None;
        }
        else if (ea.getButton() == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
        {
#ifdef HAS_VIEWER3DIN2D
            if (m_view)
            {
                if (m_mode == DragMode::DragViewport)
                {
                    m_mode = DragMode::None;
                    m_viewportIndex = -1;
                    return true;
                }
            }
#endif
        }
        break;
    case(osgGA::GUIEventAdapter::DRAG):
        if (ea.getButtonMask() == osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON)
        {
            if (m_mode == DragMode::Pan)
            {
                auto cursorX = ea.getX();
                auto cursorY = ea.getY();
                pan(cursorX - m_cursorLastX, cursorY - m_cursorLastY);
                m_cursorLastX = cursorX;
                m_cursorLastY = cursorY;
                return true;
            }
        }
        else if (ea.getButtonMask() == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
        {
#ifdef HAS_VIEWER3DIN2D
            if (m_view)
            {
                if (m_mode == DragMode::DragViewport && m_viewportIndex >= 0)
                {
                    m_view->moveViewport(m_viewportIndex, ea.getX() - m_cursorLastX, ea.getY() - m_cursorLastY);
                    m_cursorLastX = ea.getX();
                    m_cursorLastY = ea.getY();
                    return true;
                }
            }
#endif
        }
        break;
    case(osgGA::GUIEventAdapter::SCROLL):
        {
            auto sm = ea.getScrollingMotion();
            double factor = 1.0;
            if (sm == osgGA::GUIEventAdapter::SCROLL_DOWN)
                factor = 0.8;
            else if (sm == osgGA::GUIEventAdapter::SCROLL_UP)
                factor = 1.25;
            if (factor != 1.0)
            {
                zoom(factor, ea.getX(), ea.getY());
                return true;
            }
        }
        break;
    default:
        break;
    }
    return false;
}

void ZoomPanManipulator::setZoomMode(ZoomMode mode)
{
    m_zoomMode = mode;
}

void ZoomPanManipulator::setMargin(double margin)
{
    if (margin >= 0.0)
        m_margin = margin;
}

bool ZoomPanManipulator::zoomAll()
{
    auto camera = getCamera();
    auto vp = camera->getViewport();
    if (!vp)
        return false;
    if (m_camera)
    {
        auto root = m_scene;
        osg::ComputeBoundsVisitor cbv;
        root->accept(cbv);
        auto bbox = cbv.getBoundingBox();
        auto vpw = vp->width();
        auto vph = vp->height();
        auto WL = bbox.xMax() - bbox.xMin();
        auto HL = bbox.yMax() - bbox.yMin();

        double WC, HC;
        WC = vpw - 2 * m_margin;
        if (WC <= 0)
            return false;
        HC = vph - 2 * m_margin;
        if (HC <= 0)
            return false;

        double WLHC = WL * HC;
        double WCHL = WC * HL;
        double detx = 0.0, dety = 0.0;
        if (WLHC > WCHL)
        {
            m_baseZoom = WL / WC;
            double newHL = WLHC / WC;
            dety = (newHL - HL) / 2.0;
        }
        else
        {
            m_baseZoom = HL / HC;
            double newWL = WCHL / HC;
            detx = (newWL - WL) / 2.0;
        }
        detx += m_margin * m_baseZoom;
        dety += m_margin * m_baseZoom;
        camera->setProjectionMatrixAsOrtho(bbox.xMin() - detx, bbox.xMax() + detx, bbox.yMin() - dety, bbox.yMax() + dety, bbox.zMin(), bbox.zMax());
    }
#ifdef HAS_VIEWER3DIN2D
    else if (m_view)
    {
        m_baseZoom = 1;
        camera->setProjectionMatrixAsOrtho(vp->x(), vp->x() + vp->width(), vp->y(), vp->y() + vp->height(), -1, 1);
        m_view->updateViewport(vp->x(), vp->y(), 1.0);
    }
#endif
    m_zoomFactor = 1.0;
    return true;
}

void ZoomPanManipulator::zoom(double factor, float cursorX, float cursorY)
{
    assert(factor > 0.0);
    // Limit zoom between [0.1, 10] times of zoom all.
    double tmp = factor * m_zoomFactor;
    if (tmp < 0.1 || tmp > 10)
        return;

    auto camera = getCamera();
    double l, r, b, t, n, f;
    bool success = camera->getProjectionMatrixAsOrtho(l, r, b, t, n, f);
    if (success)
    {
        double zoom = m_baseZoom * m_zoomFactor;
        double fixedPointX, fixedPointY;
        if (m_zoomMode == ZoomCursor)
        {
            auto vp = camera->getViewport();
            if (!vp)
                return;
            fixedPointX = l + (cursorX - vp->x()) * zoom;
            fixedPointY = b + (cursorY - vp->y()) * zoom;
        }
        else
        {
            fixedPointX = (l + r) / 2.0;
            fixedPointY = (b + t) / 2.0;
        }
        l = fixedPointX - (fixedPointX - l) * factor;
        r = fixedPointX + (r - fixedPointX) * factor;
        b = fixedPointY - (fixedPointY - b) * factor;
        t = fixedPointY + (t - fixedPointY) * factor;
        camera->setProjectionMatrixAsOrtho(l, r, b, t, n, f);
        m_zoomFactor = tmp;
#ifdef HAS_VIEWER3DIN2D
        if (m_view)
        {
            m_view->updateViewport(l, b, m_baseZoom * m_zoomFactor);
        }
#endif
    }
}

void ZoomPanManipulator::pan(float cursorDx, float cursorDy)
{
    auto camera = getCamera();
    double l, r, b, t, n, f;
    bool success = camera->getProjectionMatrixAsOrtho(l, r, b, t, n, f);
    if (success)
    {
        double zoom = m_baseZoom * m_zoomFactor;
        double dx = cursorDx * zoom;
        double dy = cursorDy * zoom;

        l -= dx;
        r -= dx;
        b -= dy;
        t -= dy;
        camera->setProjectionMatrixAsOrtho(l, r, b, t, n, f);
#ifdef HAS_VIEWER3DIN2D
        if (m_view)
        {
            m_view->updateViewport(l, b, zoom);
        }
#endif
    }
}

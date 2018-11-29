#include "stdafx.h"
#include <assert.h>
#include <osg/ComputeBoundsVisitor>
#include <osgViewer/View>
#include <osg/Viewport>
#include "ZoomPanManipulator.h"

#if defined(USE_VIEWER3DIN2D)
#include "Viewer3Din2D.h"
#endif

ZoomPanManipulator::ZoomPanManipulator()
    : m_zoomFactor(1.0)
    , m_baseZoom(1.0)
    , m_zoomMode(ZoomCursor)
#if defined(GET_BBOX_FROM_SCENE)
    , m_margin(5)
#endif
    , m_firstTime(true)
{
}

ZoomPanManipulator::~ZoomPanManipulator()
{
}

bool ZoomPanManipulator::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
    osgViewer::View* view = dynamic_cast<osgViewer::View*>(&aa);
    if (!view)
        return false;
    // Don't disturb camera manipulator if it exists.
    if (view->getCameraManipulator())
        return false;
    auto camera = view->getCamera();
    if (!camera)
        return false;
    switch (ea.getEventType())
    {
    case (osgGA::GUIEventAdapter::FRAME):
        if (m_firstTime)
        {
            m_firstTime = false;
#if defined(USE_VIEWER3DIN2D)
            auto view3din2d = dynamic_cast<Viewer3Din2D*>(view);
            view3din2d->UpdateViewportFrames();
#endif
            camera->setViewMatrixAsLookAt(osg::Vec3d(0, 0, 1), osg::Vec3d(0, 0, 0), osg::Vec3d(0, 1, 0));
            ZoomAll(view);
        }
        break;
    case osgGA::GUIEventAdapter::DOUBLECLICK:
        if (ea.getButton() == osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON)
        {
            ZoomAll(view);
        }
        break;
    case(osgGA::GUIEventAdapter::PUSH):
        if (ea.getButton() == osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON)
        {
            m_cursorLastX = ea.getX();
            m_cursorLastY = ea.getY();
        }
        break;
    case(osgGA::GUIEventAdapter::RELEASE):
        if (ea.getButton() == osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON)
        {
        }
        break;
    case(osgGA::GUIEventAdapter::DRAG):
        if (ea.getButtonMask() == osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON)
        {
            auto cursorX = ea.getX();
            auto cursorY = ea.getY();
            move(view, cursorX - m_cursorLastX, cursorY - m_cursorLastY);
            m_cursorLastX = cursorX;
            m_cursorLastY = cursorY;
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
                Zoom(view, factor, ea.getX(), ea.getY());
            }
        }
        break;
    case(osgGA::GUIEventAdapter::KEYDOWN):
        if (ea.getKey() == osgGA::GUIEventAdapter::KEY_Escape)
        {
            return true;
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

#if defined(GET_BBOX_FROM_SCENE)
void ZoomPanManipulator::setMargin(double margin)
{
    if (margin >= 0.0)
        m_margin = margin;
}
#endif

bool ZoomPanManipulator::ZoomAll(osgViewer::View* view)
{
    if (!view)
        return false;
    auto camera = view->getCamera();
    if (!camera)
        return false;
    auto vp = camera->getViewport();
    if (!vp)
        return false;
#if defined(GET_BBOX_FROM_SCENE)
    auto root = view->getSceneData();
    if (!root)
        return false;
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
#else
    m_baseZoom = 1;
    camera->setProjectionMatrixAsOrtho(vp->x(), vp->x() + vp->width(), vp->y(), vp->y() + vp->height(), -1, 1);
#endif

    m_zoomFactor = 1.0;
#if defined(USE_VIEWER3DIN2D)
    auto view3din2d = dynamic_cast<Viewer3Din2D*>(view);
    view3din2d->UpdateViewport(vp->x(), vp->y(), 1.0);
#endif
    return true;
}

bool ZoomPanManipulator::DPtoLP(osg::Camera* camera, const osg::Vec2 & dp, osg::Vec2& lp)
{
    double l, r, b, t, n, f;
    bool success = camera->getProjectionMatrixAsOrtho(l, r, b, t, n, f);
    if (success)
    {
        auto vp = camera->getViewport();
        if (vp)
        {
            double zoom = m_baseZoom * m_zoomFactor;
            lp.x() = l + (dp.x() - vp->x()) * zoom;
            lp.y() = b + (dp.y() - vp->y()) * zoom;
            return true;
        }
    }
    return false;
}

bool ZoomPanManipulator::DPtoLP(osg::Camera * camera, const osg::Vec2Array & dps, osg::Vec2Array & lps)
{
    double l, r, b, t, n, f;
    bool success = camera->getProjectionMatrixAsOrtho(l, r, b, t, n, f);
    if (success)
    {
        auto vp = camera->getViewport();
        if (vp)
        {
            double zoom = m_baseZoom * m_zoomFactor;
            lps.resize(dps.size());
            for (size_t i = 0; i<dps.size(); ++i)
            {
                auto& lp = lps[i];
                const auto& dp = dps[i];
                lp.x() = l + (dp.x() - vp->x()) * zoom;
                lp.y() = b + (dp.y() - vp->y()) * zoom;
            }
            return true;
        }
    }
    return false;
}

void ZoomPanManipulator::Zoom(osgViewer::View* view, double factor, float cursorX, float cursorY)
{
    assert(factor > 0.0);
    // Limit zoom between [0.1, 10] times of zoom all.
    double tmp = factor * m_zoomFactor;
    if (tmp < 0.1 || tmp > 10)
        return;

    auto camera = view->getCamera();
    double l, r, b, t, n, f;
    bool success = camera->getProjectionMatrixAsOrtho(l, r, b, t, n, f);
    if (success)
    {
        double zoom = m_baseZoom * m_zoomFactor;
        double fixedPointX, fixedPointY;
        if (m_zoomMode == ZoomCursor)
        {
            auto camera = view->getCamera();
            if (!camera)
                return;
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
#if defined(USE_VIEWER3DIN2D)
        auto view3din2d = dynamic_cast<Viewer3Din2D*>(view);
        view3din2d->UpdateViewport(l, b, m_baseZoom * m_zoomFactor);
#endif
    }
}

void ZoomPanManipulator::move(osgViewer::View * view, float cursorDx, float cursorDy)
{
    auto camera = view->getCamera();
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
#if defined(USE_VIEWER3DIN2D)
        auto view3din2d = dynamic_cast<Viewer3Din2D*>(view);
        view3din2d->UpdateViewport(l, b, zoom);
#endif
    }
}

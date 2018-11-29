#pragma once
#include <osgGA/GUIEventHandler>
namespace osgViewer
{
class View;
}

#if !defined(GET_BBOX_FROM_SCENE)
#define USE_VIEWER3DIN2D
#endif

// This class is like a camera manipulator which can zoom or pan the scene of master camera.
// This class can be used outside of this project by define GET_BBOX_FROM_SCENE
class ZoomPanManipulator : public osgGA::GUIEventHandler
{
public:
    ZoomPanManipulator();
    ~ZoomPanManipulator();
    //META_Object(osgGA, ZoomPanManipulator)
    virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa) override;
    enum ZoomMode {ZoomCursor, ZoomCenter};
    void setZoomMode(ZoomMode mode);
#if defined(GET_BBOX_FROM_SCENE)
    void setMargin(double margin/*pixels*/);
#endif
    bool ZoomAll(osgViewer::View* view);
private:
    void Zoom(osgViewer::View* view, double factor, float cursorX, float cursorY);
    void move(osgViewer::View* view, float cursorDx, float cursorDy);
    double m_zoomFactor;
    double m_baseZoom;
    ZoomMode m_zoomMode;
#if defined(GET_BBOX_FROM_SCENE)
    double m_margin; // in pixels
#endif
    float m_cursorLastX;
    float m_cursorLastY;
    bool m_firstTime;
};


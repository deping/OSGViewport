#pragma once
#include <osgGA/GUIEventHandler>
namespace osgViewer
{
class View;
}

#define HAS_VIEWER3DIN2D

// This class is like a camera manipulator which can zoom or pan the scene of camera.
// If it is used in Viewer3Din2D master camera, it can drag viewports.
// This class can be used outside of this project by removing definition of HAS_VIEWER3DIN2D
#ifdef HAS_VIEWER3DIN2D
class Viewer3Din2D;
#endif
class ZoomPanManipulator : public osgGA::GUIEventHandler
{
public:
    // Programmer must choose one of the two constructors.
#ifdef HAS_VIEWER3DIN2D
    ZoomPanManipulator(Viewer3Din2D* view);
#endif
    ZoomPanManipulator(osg::Camera* camera, osg::Node* scene);
    ~ZoomPanManipulator();
    //META_Object(osgGA, ZoomPanManipulator)
    virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa) override;
    enum ZoomMode {ZoomCursor, ZoomCenter};
    void setZoomMode(ZoomMode mode);
    void setMargin(double margin/*pixels*/);
    bool zoomAll();
private:
    osg::Camera* getCamera() const;
    void zoom(double factor, float cursorX, float cursorY);
    void pan(float cursorDx, float cursorDy);
    double m_zoomFactor;
    double m_baseZoom;
    ZoomMode m_zoomMode;
    double m_margin; // in pixels
    float m_cursorLastX;
    float m_cursorLastY;
    bool m_firstTime;
    enum class DragMode { None, DragViewport, Pan };
    DragMode m_mode;
    // Either m_view or m_camera is valid.
#ifdef HAS_VIEWER3DIN2D
    Viewer3Din2D* m_view;
    int m_viewportIndex;
#endif
    osg::Camera* m_camera;
    osg::Node* m_scene;
};


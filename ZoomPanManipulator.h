#pragma once
#include <osgGA/GUIEventHandler>
namespace osgViewer
{
class View;
}

#define HAS_VIEWER3DIN2D

// This class is like a camera manipulator which can zoom or pan the scene of master camera.
// This class can be used outside of this project by define HAS_VIEWER3DIN2D
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
    bool ZoomAll();
private:
    osg::Camera* getCamera() const;
    void Zoom(double factor, float cursorX, float cursorY);
    void move(float cursorDx, float cursorDy);
    double m_zoomFactor;
    double m_baseZoom;
    ZoomMode m_zoomMode;
    double m_margin; // in pixels
    float m_cursorLastX;
    float m_cursorLastY;
    bool m_firstTime;
    // Either m_view or m_camera is valid.
#ifdef HAS_VIEWER3DIN2D
    Viewer3Din2D* m_view;
#endif
    osg::Camera* m_camera;
    osg::Node* m_scene;
};


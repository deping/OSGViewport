#pragma once

#include <vector>

#include <osg/MatrixTransform>
#include <osgViewer/Viewer>

struct ViewportDim
{
    double x;
    double y;
    double width;
    double height;
};

class ViewportActivator;
class ZoomPanManipulator;
class ViewportFrame;
class Viewer3Din2D : public osgViewer::Viewer
{
public:
    using base = osgViewer::Viewer;
    Viewer3Din2D();
    ~Viewer3Din2D();
    // if cameraManipulator==nullptr, TrackballManipulator will be used.
    // cameraManipulator can be instance of ZoomPanManipulator or osgGA::CameraManipulator
    // sceneGraph is scene graph of this slave camera.
    // follower must be in scene graph of master camera.
    // slaves should have different followers.
    bool addSlave(osg::Camera* camera, osg::Group* sceneGraph, osgGA::GUIEventHandler* cameraManipulator = nullptr, osg::MatrixTransform* follower = nullptr);
    virtual void realize() override;

private:
    // i == -1, Enable master camera manipulator
    // i >= 0, Enable ith slave camera manipulator
    // one and only one manipulator is active.
    // // return true if it deactivates and/or activates some slave viewport.
    bool activateCameraManipulator(int i, bool activate);
    using ChangeEventCallback = void(osg::Node::*)(osg::Callback*);
    void enableCameraManipulator(int i, ChangeEventCallback changeEventCallback, float linewidth);
    int viewportHit(double x, double y);
    // Can only be called when viewport dimension is the same as its logical dimensions.
    void initViewportFrames();
    void updateViewport(double l, double b, double zoom);
    void moveViewport(int i, double dx, double dy);

    friend class ViewportActivator;
    friend class ZoomPanManipulator;
    friend struct FixedSizeSlaveViewport_ResizedCallback;
    osg::ref_ptr<ViewportActivator> m_viewportActivator;
    osg::ref_ptr<ZoomPanManipulator> m_masterCameraManipulator;
    // size = slave size
    std::vector<osg::ref_ptr<osg::MatrixTransform>> m_followers;
    // logical dimensions of viewport, keep unchaged after initialization or resizing
    std::vector<ViewportDim> m_viewportDims;
    std::vector<ViewportFrame*> m_viewportFrames;
    // -1 , master camera manipulator is active
    // >=0 , ith slave camera manipulator is active
    int m_activeManipulatorIndex;
};


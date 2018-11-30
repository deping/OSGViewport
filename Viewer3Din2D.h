#pragma once

#include <vector>
#include <osgViewer/Viewer>

struct ViewportDim
{
    double x;
    double y;
    double width;
    double height;
};

class MasterCameraHandler;
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
    bool addSlave(osg::Camera* camera, osg::Group* sceneGraph, osgGA::GUIEventHandler* cameraManipulator = nullptr);
    virtual void realize() override;

private:
    // i == -1, Enable master camera manipulator
    // i >= 0, Enable ith slave camera manipulator
    // one and only one manipulator is active.
    // // return true if it deactivates and/or activates some slave viewport.
    bool ActivateCameraManipulator(int i, bool activate);
    using ChangeEventCallback = void(osg::Node::*)(osg::Callback*);
    void EnableCameraManipulator(int i, ChangeEventCallback changeEventCallback, float linewidth);
    int ViewportHit(double x, double y);
    // Can only be called when viewport dimension is the same as its logical dimensions.
    void UpdateViewportFrames();
    void UpdateViewport(double l, double b, double zoom);
    void MoveViewport(int i, double dx, double dy);

    friend class MasterCameraHandler;
    friend class ZoomPanManipulator;
    osg::ref_ptr<MasterCameraHandler> m_masterCameraHandler;
    osg::ref_ptr<ZoomPanManipulator> m_masterCameraManipulator;
    // size = slave size
    std::vector<osg::ref_ptr<osgGA::GUIEventHandler>> m_cameraManipulators;
    // logical dimensions of viewport, keep unchaged after initialization
    std::vector<ViewportDim> m_viewportDims;
    std::vector<ViewportFrame*> m_viewportFrames;
    // -1 , master camera manipulator is active
    // >=0 , ith slave camera manipulator is active
    int m_activeManipulatorIndex;
};


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
    bool addSlave(osg::Camera* camera, osg::Group* sceneGraph, osgGA::CameraManipulator* cameraManipulator = nullptr);
    virtual void realize() override;

private:
    // i == -1, Enable master camera manipulator
    // i >= 0, Enable ith slave camera manipulator
    // one and only one manipulator is active.
    bool ActivateCameraManipulator(int i, bool activate);
    using ChangeEventCallback = void(osg::Node::*)(osg::Callback*);
    void EnableCameraManipulator(int i, ChangeEventCallback changeEventCallback, float linewidth);
    int ViewportHit(double x, double y);
    void UpdateViewportFrames(/*ZoomPanManipulator* zoom*/);
    void UpdateViewport(double l, double b, double zoom);

    friend class MasterCameraHandler;
    friend class ZoomPanManipulator;
    osg::ref_ptr<MasterCameraHandler> m_masterCameraHandler;
    osg::ref_ptr<ZoomPanManipulator> m_masterCameraManipulator;
    // size = slave size
    std::vector<osg::ref_ptr<osgGA::CameraManipulator>> m_cameraManipulators;
    // keep unchaged after initializatioin
    std::vector<ViewportDim> m_viewportDims;
    std::vector<ViewportFrame*> m_viewportFrames;
    int m_activeManipulatorIndex;
};


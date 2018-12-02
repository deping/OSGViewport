# Sizable and Movable Viewports
Embed **Sizable and Movable** 3D/2D viewports in 2D viewport; Each viewport have its independent camera manipulator. The master 2D viewport's camera manipulator can only zoom and pan view, can't rotate view.

![OsgViewport picture](https://github.com/deping/images/blob/master/OSGViewport.png)

The only API is class Viewer3Din2D:
```C++
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
    ...
};
```

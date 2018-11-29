# Sizable and Movable Viewports
Embed **Sizable and Movable** 3D viewports in 2D viewport; Each viewport have its independent camera manipulator. The master 2D viewport's camera manipulator can only zoom and pan, can't rotate view.
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
    bool addSlave(osg::Camera* camera, osg::Group* sceneGraph, osgGA::CameraManipulator* cameraManipulator = nullptr);
    ...
};
```

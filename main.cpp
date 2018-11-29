#include "stdafx.h"
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/MatrixTransform>
#include <osg/ShapeDrawable>
#include <osgViewer/ViewerEventHandlers>
#include <osg/LineWidth>
#include "Viewer3Din2D.h"

int main(int, char**)
{
    Viewer3Din2D viewer;
    //viewer.setUpViewInWindow(100, 100, 600, 800);
    osg::ref_ptr<osg::GraphicsContext::Traits> traits =
        new osg::GraphicsContext::Traits;
    traits->x = 50;
    traits->y = 50;
    traits->width = 800;
    traits->height = 600;
    traits->windowDecoration = true;
    traits->doubleBuffer = true;
    //traits->samples = 4;

    osg::ref_ptr<osg::GraphicsContext> gc =
        osg::GraphicsContext::createGraphicsContext(traits.get());
    osg::ref_ptr<osg::Camera> camera = viewer.getCamera();
    camera->setGraphicsContext(gc);
    camera->setViewport(
        new osg::Viewport(0, 0, traits->width, traits->height));
    camera->setClearMask(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    camera->setClearColor(osg::Vec4f(0.2f, 0.2f, 0.4f, 1.0f));

    osg::ref_ptr<osg::Group> root = new osg::Group();
    viewer.setSceneData(root.get());

#if 1
    osg::ref_ptr<osg::MatrixTransform> tf = new osg::MatrixTransform;
    tf->setMatrix(osg::Matrix::translate(3, 4, 0));
    auto geode = new osg::Geode;
    auto box = new osg::ShapeDrawable(new osg::Box(osg::Vec3(traits->width * 3 / 4, traits->height / 2, 0), traits->width/2-10, traits->height/2-10, 1));
    geode->addDrawable(box);
    tf->addChild(geode);
    root->addChild(tf);
#endif

    osg::ref_ptr<osg::Camera> camera2 = new osg::Camera;
    camera2->setViewport(
        new osg::Viewport(100, 50, traits->width/2, traits->height/2));
    camera2->setClearMask(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    camera2->setClearColor(osg::Vec4f(0.4f, 0.2f, 0.2f, 1.0f));

    osg::ref_ptr<osg::Group> root2 = new osg::Group();
    osg::ref_ptr<osg::MatrixTransform> tf2 = new osg::MatrixTransform;
    auto geode2 = new osg::Geode;
    auto box2 = new osg::ShapeDrawable(new osg::Cone());
    geode2->addDrawable(box2);
    tf2->addChild(geode2);
    root2->addChild(tf2);

    viewer.addSlave(camera2.get(), root2.get());

    osg::ref_ptr<osg::Camera> camera3 = new osg::Camera;
    camera3->setViewport(
        new osg::Viewport(100, 400, traits->width/2, traits->height/2));
    camera3->setClearMask(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    camera3->setClearColor(osg::Vec4f(0.2f, 0.4f, 0.2f, 1.0f));

    viewer.addSlave(camera3.get(), root2.get());

    // root->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
    // root->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);
    // root->getOrCreateStateSet()->setMode(GL_LINE_SMOOTH, osg::StateAttribute::ON);
    // add the state manipulator
    //viewer.addEventHandler(new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()));

    // add the stats handler
    viewer.addEventHandler(new osgViewer::StatsHandler);
    // osg::ref_ptr<osgGA::TrackballManipulator> man = new osgGA::TrackballManipulator();
    // man->setHomePosition(osg::Vec3d(5, 5, 20), osg::Vec3d(5, 5, 0), osg::Vec3d(0, 1, 0));
    // viewer.setCameraManipulator(man.get());
    // viewer.realize();

    viewer.setRunFrameScheme(osgViewer::Viewer::ON_DEMAND);
    return viewer.run();
}

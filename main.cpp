#include "stdafx.h"

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/LineWidth>
#include <osg/MatrixTransform>
#include <osg/ShapeDrawable>
#include <osgText/Text>
#include <osgViewer/ViewerEventHandlers>

#include "Viewer3Din2D.h"
#include "ZoomPanManipulator.h"

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
    auto strFontFileCn = "C:\\Windows\\Fonts\\simsun.ttc";
    auto font = osgText::readRefFontFile(strFontFileCn);

    osg::ref_ptr<osg::MatrixTransform> tf = new osg::MatrixTransform;
    auto geode = new osg::Geode;
    auto leftText = new osgText::Text;
    leftText->setText(L"福如东海");
    leftText->setCharacterSize(30);
    leftText->setFont(font);
    leftText->setRotation(osg::Quat(osg::PI_2, osg::Z_AXIS));
    leftText->setPosition(osg::Vec3d(100 - 5, 400, 0));
    geode->addDrawable(leftText);
    auto rightText = new osgText::Text;
    rightText->setText(L"寿比南山");
    rightText->setCharacterSize(30);
    rightText->setFont(font);
    rightText->setRotation(osg::Quat(osg::PI_2, osg::Z_AXIS));
    rightText->setPosition(osg::Vec3d(100 + traits->width / 2 + 30 + 5, 400, 0));
    geode->addDrawable(rightText);
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

    viewer.addSlave(camera3.get(), root2.get(), new ZoomPanManipulator(camera3.get(), root2.get()), tf);

    // add the stats handler
    viewer.addEventHandler(new osgViewer::StatsHandler);

    viewer.setRunFrameScheme(osgViewer::Viewer::ON_DEMAND);
    return viewer.run();
}

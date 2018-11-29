#pragma once
#include <string>
#include <vector>
#include <osg/Geometry>
#include <osg/BoundingBox>

class ZoomPanManipulator;
namespace osg
{
class Viewport;
}
class ViewportFrame : 
	public osg::Geometry
{
public:
	ViewportFrame();
	/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
	ViewportFrame(const ViewportFrame& pg, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY);
	~ViewportFrame();
    virtual osg::Object* cloneType() const { return new ViewportFrame(); }
    virtual osg::Object* clone(const osg::CopyOp& copyop) const { return new ViewportFrame(*this, copyop); }
    virtual bool isSameKindAs(const osg::Object* obj) const { return dynamic_cast<const ViewportFrame*>(obj) != NULL; }
    virtual const char* className() const { return "ViewportFrame"; }
    virtual const char* libraryName() const { return "osg"; }

	// Ensure color component is in [0, 1]
    void setColor(const osg::Vec3& color);
    const osg::Vec3& getColor() const { return _color; }
    void setRect(/*ZoomPanManipulator* zoom, */osg::Camera* camera);

    osg::BoundingBox computeBoundingBox() const;

    void drawImplementation(osg::RenderInfo& renderInfo) const;

private:

    // members which have public access.
    osg::Vec3                               _color;
    float                                   _lineWidth;
	osg::ref_ptr<osg::DrawArrays> m_primitiveSet;
	osg::ref_ptr<osg::Vec2Array> m_rect;
    osg::ref_ptr<osg::Vec3Array>    _colors;
};

#include "stdafx.h"

#include <assert.h>

#include <osg/GLExtensions>
#include <osg/LineWidth>
#include <osg/PrimitiveSet>
#include <osg/State>
#include <osg/Viewport>
#include <osgUtil/CullVisitor>

#include "ViewportFrame.h"
#include "ZoomPanManipulator.h"

#undef DrawText

ViewportFrame::ViewportFrame(void)
    : _color(1, 1, 1)
    , _lineWidth(1.0f)
    , m_primitiveSet(new osg::DrawArrays(GL_LINE_LOOP, 0, 4))
    , m_rect(new osg::Vec2Array(osg::Array::BIND_PER_VERTEX))
    , _colors(new osg::Vec3Array(osg::Array::BIND_OVERALL))
{
    setVertexArray(m_rect);
    setColorArray(_colors);
    addPrimitiveSet(m_primitiveSet);
    setUseVertexBufferObjects(true);
    getOrCreateStateSet()->setAttributeAndModes(new osg::LineWidth(1.0f), osg::StateAttribute::PROTECTED | osg::StateAttribute::ON);
    _colors->push_back(_color);
    _colors->dirty();
}

ViewportFrame::ViewportFrame(const ViewportFrame& st, const osg::CopyOp& copyop)
	: Geometry(st, copyop)
    , _color(st._color)
    , _lineWidth(st._lineWidth)
    , m_primitiveSet(static_cast<osg::DrawArrays*>(copyop(st.m_primitiveSet)))
    , m_rect(static_cast<osg::Vec2Array*>(copyop(st.m_rect)))
    , _colors(static_cast<osg::Vec3Array*>(copyop(st._colors)))
{
    setVertexArray(m_rect);
    setColorArray(_colors);
    addPrimitiveSet(m_primitiveSet);
    setUseVertexBufferObjects(true);
}

ViewportFrame::~ViewportFrame(void)
{
}

void ViewportFrame::setColor(const osg::Vec3& color)
{
    if (_color != color)
    {
        _color = color;
        (*_colors)[0] = _color;
        _colors->dirty();
    }
}

void ViewportFrame::setRect(/*ZoomPanManipulator * zoom, */osg::Camera* camera)
{
    osg::Viewport * vp = camera->getViewport();
    m_rect->resize(0);
    m_rect->reserve(4);
    float x = vp->x() - 2;
    float y = vp->y() - 2;
    float w = vp->width() + 3;
    float h = vp->height() + 3;
    m_rect->push_back(osg::Vec2(x, y));
    m_rect->push_back(osg::Vec2(x + w, y));
    m_rect->push_back(osg::Vec2(x + w, y + h));
    m_rect->push_back(osg::Vec2(x, y + h));
    m_rect->dirty();
}

void ViewportFrame::drawImplementation(osg::RenderInfo& renderInfo) const
{
    Geometry::drawImplementation(renderInfo);
}

osg::BoundingBox ViewportFrame::computeBoundingBox() const
{
    //Geometry::computeBoundingBox();
    osg::BoundingBox  bbox;

    for (size_t i = 0; i < m_rect->size(); ++i)
    {
        bbox.expandBy(osg::Vec3f((*m_rect)[i], 0.f));
    }

    return bbox;
}

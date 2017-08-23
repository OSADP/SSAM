/*------------------------------------------------------------------------------
   Copyright © 2016-2017
   New Global Systems for Intelligent Transportation Management Corp.
   
   This file is part of SSAM.
  
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU Affero General Public License as
   published by the Free Software Foundation, either version 3 of the
   License, or (at your option) any later version.
  
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Affero General Public License for more details.
  
   You should have received a copy of the GNU Affero General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
------------------------------------------------------------------------------*/
#define _USE_MATH_DEFINES
#include "SSAMOSG.h"
#include <cmath>
#include <iterator>
#include <osgUtil/DelaunayTriangulator>
#include <osgUtil/Optimizer>

SSAMOSG::SSAMOSG(HWND hWnd, HWND hMapDlg)
	: m_hWnd(hWnd) 
	, m_hMapDlg(hMapDlg)
	, m_ResetFlag(0x00)
	, m_ConflictScale (1.0)
	, m_ConflictSize (1.0)
	, m_ColorCategory (0) 
	, m_pPickParam(std::make_shared<PickParam>())
	, m_FilteredTileRoot (NULL)
	, m_MapType(0)
	, m_GridSize(100)
	, m_OrigGridSize(m_GridSize)
	, m_NLevels(2)
	, m_OrigNLevels (m_NLevels)
	, m_NXGrids(0)
	, m_NYGrids(0)
	, m_MinIntervalVal(INT_MAX)
	, m_MaxIntervalVal(0)
	, m_NXTiles(0)
	, m_NYTiles(0)
	, m_Feet2OSG (0.0001)
	, m_Epsilon(1e-6)
	, m_LineWidth(0.36 * m_Feet2OSG)
	, m_ThickNessofRoad (2.0 * m_Feet2OSG)
	, m_HeightAbove (0.1 * m_Feet2OSG + m_ThickNessofRoad)
	, m_SmallestX (INT_MAX)
	, m_SmallestY (INT_MAX)
	, m_LargestX (INT_MIN)
	, m_LargestY (INT_MIN)
	, m_SmallestZ (INT_MAX)
	, m_LargestZ (INT_MIN)
	, m_ExtremeZ (0)

{
	m_Roots.resize(NUM_OF_ROOT_TYPES, NULL);
	
	for (int i = 0; i < Conflict::NUM_CONFLICT_TYPES - 1; ++i)
	{
		m_ShapeTypes.push_back(i);
	}
	
	// initial color for all TTC to white
	std::vector<double> cv(4, 1.0);
	for (int i = 0; i < NUM_TTC_LEVELS; ++i)
	{
		m_ConflictColorVecs.push_back(cv);
	}

	m_IntervalColorVecs.resize(5, cv);

	std::string fontFile("fonts/arial.ttf");
    m_Font = osgText::readFontFile(fontFile);
	m_TextColor = osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f);

	m_Delta = 3.0 * m_Feet2OSG;
}

SSAMOSG::~SSAMOSG(void)
{
	m_Viewer->setDone(true);
    Sleep(1000);
    m_Viewer->stopThreading();

    delete m_Viewer;
}

void SSAMOSG::InitOSG()
{
	m_ModelName = "";
	InitManipulators();
	InitOSGViewer();
}

void SSAMOSG::InitManipulators(void)
{
    // Create a trackball manipulator
    m_Trackball = new osgGA::TrackballManipulator();

    // Create a Manipulator Switcher
    m_KeyswitchManipulator = new osgGA::KeySwitchMatrixManipulator;

    // Add trackball manipulator to the switcher
    m_KeyswitchManipulator->addMatrixManipulator( '1', "Trackball", m_Trackball.get());

    // Init the switcher to the first manipulator (in this case the only manipulator)
    m_KeyswitchManipulator->selectMatrixManipulator(0);  // Zero based index Value
}

void SSAMOSG::InitOSGViewer(void)
{
    RECT rect;
	::GetWindowRect(m_hWnd, &rect);
	osg::ref_ptr<osg::Referenced> windata = new osgViewer::GraphicsWindowWin32::WindowData(m_hWnd);
	// Init the GraphicsContext Traits
    osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
    traits->x = 0;
    traits->y = 0;
    traits->width = rect.right - rect.left;
    traits->height = rect.bottom - rect.top;
    traits->windowDecoration = false;
    traits->doubleBuffer = true;
    traits->sharedContext = 0;
    traits->setInheritedWindowPixelFormat = true;
    traits->inheritedWindowData = windata;

	// Create the Graphics Context
    osg::GraphicsContext* gc = osg::GraphicsContext::createGraphicsContext(traits.get());

    osg::ref_ptr<osg::Camera> camera = new osg::Camera;
    camera->setGraphicsContext(gc);
	camera->setViewport(new osg::Viewport(traits->x, traits->y, traits->width, traits->height));
	
	GLenum buffer = traits->doubleBuffer ? GL_BACK : GL_FRONT;
	camera->setDrawBuffer(buffer);
	camera->setReadBuffer(buffer);
    camera->setClearMask(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    camera->setProjectionMatrixAsPerspective(
        30.0f, static_cast<double>(traits->width)/static_cast<double>(traits->height), 1.0, 1000.0);
	camera->setClearColor(osg::Vec4f(0.0f,0.3922f,0.0f, 1.0f));

   
    m_Viewer = new osgViewer::Viewer();
    m_Viewer->addEventHandler(new osgViewer::StatsHandler);
	m_Viewer->getCamera()->setGraphicsContext(gc);
	m_Viewer->getCamera()->setViewport(new osg::Viewport(traits->x, traits->y, traits->width, traits->height));
	m_Viewer->getCamera()->setClearColor(osg::Vec4f(0.0f,0.3922f,0.0f, 1.0f));
	osg::ref_ptr<SaveImageDrawCallback> saveImageDrawCallback = new SaveImageDrawCallback();
    m_Viewer->getCamera()->setPostDrawCallback (saveImageDrawCallback.get());
	m_Viewer->getCamera()->setCullMask(0x2);
	m_Viewer->setCameraManipulator(m_KeyswitchManipulator.get());

    m_RootNode = new osg::MatrixTransform;
    m_RootNode->setMatrix(osg::Matrix::rotate(osg::inDegrees(90.0f),1.0f,0.0f,0.0f));

	for (std::vector<osg::ref_ptr<osg::Group> >::iterator it = m_Roots.begin();
		it != m_Roots.end(); ++it)
	{
		(*it) = new osg::Group;
		// Disable lighting in the root node's StateSet. Make
		//   it PROTECTED to prevent osgviewer from enabling it.
		osg::ref_ptr<osg::StateSet> state = (*it)->getOrCreateStateSet();
		state->setMode( GL_LIGHTING,
			osg::StateAttribute::OFF |
			osg::StateAttribute::PROTECTED );
		m_RootNode->addChild(*it);
	}
	
    // run optimization over the scene graph
    osgUtil::Optimizer optimzer;
    optimzer.optimize(m_RootNode);

	m_Viewer->addEventHandler(new PickHandler(m_pPickParam));
    m_Viewer->setSceneData(m_RootNode);

	m_CameraMTF = new osg::MatrixTransform;
	m_CameraMTF->setMatrix(osg::Matrix::rotate(osg::inDegrees(90.0f),1.0f,0.0f,0.0f));

	m_FilteredTileRoot = new osg::Group;
	
    // Realize the Viewer
    m_Viewer->realize();
}

void SSAMOSG::SetSceneGraph(std::string filename, const std::list<SP_Conflict>& conflictList, bool isNewCase)
{
	m_ModelName = filename;
	m_ConflictList = std::vector<SP_Conflict>(conflictList.begin(), conflictList.end());;
	m_IsNewCase = isNewCase;
	m_ResetFlag |= 0x0080;
}

void SSAMOSG::RenderSceneGraph(std::string modelname, const std::vector<SP_Conflict>& conflictList, bool isNewCase)
{
	m_ModelName = modelname;
	DrawMap(m_Roots[MAPROOT]);
	if (!conflictList.empty())
	{
		CreateColorVectors();
		// create the roadway network and assign to m_Roots[NETWORKROOT]
		DrawConflicts(m_Roots[CONFLICTROOT]);
		DrawTiles( m_Roots[TILEROOT], 
			m_FilteredTileIndexes, m_FilteredTileRoot);
		DrawGrids(m_Roots[GRIDROOT], m_Roots[BARCHARTROOT]);
		DrawContourMap(m_Roots[CONTOURROOT], m_Roots[HEATROOT]);
	}
	SetRootMask();

	if (m_FilteredTileRoot->getNumChildren() > 0)
	{
		m_CameraMTF->removeChild(m_Roots[CONFLICTROOT]);
		m_CameraMTF->addChild(m_FilteredTileRoot);
	} else
	{
		m_CameraMTF->removeChild(m_FilteredTileRoot);
		m_CameraMTF->addChild(m_Roots[CONFLICTROOT]);
	}
	ResetViewer();
}


void SSAMOSG::EditMap(const std::string& f)
{
	SetMapFile(f);
	m_ResetFlag |= 0x0100;
}

void SSAMOSG::ResetMapType(int mt)
{
	m_MapType = mt;
	SetRootMask();
}

void SSAMOSG::ResetConflictScale(double cs) 
{
	m_ConflictScale = cs;
	m_ResetFlag |= 0x0001;
}

void SSAMOSG::ResetConflictProp(double scale, 
		const std::vector<int>& shapeTypes,
		int colorCategory,
		const std::vector<std::vector<double> >& colorVecs,
		char iResetFlag)
{
	if ((iResetFlag & 0x0001) == 0x01)
	{
		m_ConflictScale = scale;
	}

	if ((iResetFlag & 0x0002) == 0x02)
	{
		m_OrigShapeTypes = m_ShapeTypes;
		m_ShapeTypes = shapeTypes;
	}

	if ((iResetFlag & 0x0004) == 0x04)
	{
		m_ColorCategory = colorCategory;
		m_ConflictColorVecs = colorVecs;
	}

	m_ResetFlag = iResetFlag;
}

void SSAMOSG::ResetGridSize(double gs)
{
	m_OrigGridSize = m_GridSize;
	
	if (m_OrigGridSize != gs)
	{
		m_GridSize = gs * m_Feet2OSG;
		m_ResetFlag |= 0x0010;
	}
}

void SSAMOSG::ResetNLevels(int n)
{
	m_OrigNLevels = m_NLevels;

	if (m_OrigNLevels != n)
	{
		m_NLevels = n;		
		m_ResetFlag |= 0x20;
	} 
}

void SSAMOSG::ResetIntervalColors(const std::vector<std::vector<double> >& ICVs)
{
	SetIntervalColorVecs(ICVs);
	m_ResetFlag |= 0x40;
}

void SSAMOSG::ResetMapCoords(double c[])
{
	SetMapCoords(c);
	m_ResetFlag |= 0x0008;
}

void SSAMOSG::ResetColors(int cc, const std::vector<std::vector<double> >& CCVs)
{
	m_ColorCategory = cc;
	m_ConflictColorVecs = CCVs;
	m_ResetFlag |= 0x0004;
}

// redraw methods
void SSAMOSG::RedrawScale()
{
	double halfScale = m_ConflictScale / 2.0;
	for (unsigned int i = 0; i < m_Roots[CONFLICTROOT]->getNumChildren(); ++i)
	{
		osg::Geode* geoNode = (osg::Geode*)(m_Roots[CONFLICTROOT]->getChild(i));
		osg::ShapeDrawable* shapeDrawable = (osg::ShapeDrawable*)(geoNode->getDrawable(0));
		osg::Shape* shape = shapeDrawable->getShape();
		
		std::string name(geoNode->getName());
		std::size_t spos = name.find_first_of("_");
		std::size_t epos = name.find_first_of( "_", spos+1);
		
		std::string tmpstr(name.substr(spos+1, epos-spos-1));
		int idx = stoi(tmpstr);
		spos = epos;
		epos = name.find_first_of( "_", spos+1);
		tmpstr = name.substr(spos+1, epos-spos-1);
		int conflictType = stoi(tmpstr);

		int shapeType = m_ShapeTypes[conflictType];
		switch (shapeType)
		{
		case 0:
			{
			osg::Cone* curShape = (osg::Cone*)shape;
			curShape->setRadius(halfScale);
			curShape->setHeight(m_ConflictScale);
			break;
			}
		case 1:
			{
			osg::Box* curShape = (osg::Box*)shape;
			osg::Vec3 halfLengths = osg::Vec3(halfScale, halfScale, halfScale);
			curShape->setHalfLengths(halfLengths);
			break;
			}
		case 2:
			{
			osg::Sphere* curShape = (osg::Sphere*)shape;
			curShape->setRadius(halfScale);
			break;
			}
		}
		
		shapeDrawable->dirtyDisplayList();
	}
}

void SSAMOSG::RedrawShape()
{
	double halfScale = m_ConflictScale / 2.0;
	for (unsigned int i = 0; i < m_Roots[CONFLICTROOT]->getNumChildren(); ++i)
	{
		osg::Geode* geoNode = (osg::Geode*)(m_Roots[CONFLICTROOT]->getChild(i));
		osg::ShapeDrawable* shapeDrawable = (osg::ShapeDrawable*)(geoNode->getDrawable(0));
		osg::Shape* shape = shapeDrawable->getShape();
		
		std::string name(geoNode->getName());
		std::size_t spos = name.find_first_of("_");
		std::size_t epos = name.find_first_of( "_", spos+1);
		
		std::string tmpstr(name.substr(spos+1, epos-spos-1));
		int idx = stoi(tmpstr);
		spos = epos;
		epos = name.find_first_of( "_", spos+1);
		tmpstr = name.substr(spos+1, epos-spos-1);
		int conflictType = stoi(tmpstr);
		
		spos = name.find_last_of("_");
		double TTC = stoi(name.substr(spos+1));

		
		osg::Vec3 center;
		int shapeType = m_OrigShapeTypes[conflictType];
		switch (shapeType)
		{
		case 0:
			{
			osg::Cone* curShape = (osg::Cone*)shape;
			center = curShape->getCenter();
			break;
			}
		case 1:
			{
			osg::Box* curShape = (osg::Box*)shape;
			center = curShape->getCenter();
			break;
			}
		case 2:
			{
			osg::Sphere* curShape = (osg::Sphere*)shape;
			center = curShape->getCenter();
			break;
			}
		}

		shapeType = m_ShapeTypes[conflictType];
		switch (shapeType)
		{
		case 0:
			shape = new osg::Cone(center,halfScale, m_ConflictScale);
			break;
		case 1:
			shape = new osg::Box(center,m_ConflictScale);
			break;
		case 2:
			shape = new osg::Sphere(center,halfScale);
			break;
		}
		shapeDrawable->setShape(shape);
		shapeDrawable->dirtyDisplayList();
	}
}

void SSAMOSG::RedrawColor()
{
	double halfScale = m_ConflictScale / 2.0;
	for (unsigned int i = 0; i < m_Roots[CONFLICTROOT]->getNumChildren(); ++i)
	{
		osg::Geode* geoNode = (osg::Geode*)(m_Roots[CONFLICTROOT]->getChild(i));
		osg::ShapeDrawable* shapeDrawable = (osg::ShapeDrawable*)(geoNode->getDrawable(0));
		osg::Shape* shape = shapeDrawable->getShape();
		
		std::string name(geoNode->getName());
		std::size_t spos = name.find_first_of("_");
		std::size_t epos = name.find_first_of( "_", spos+1);
		
		std::string tmpstr(name.substr(spos+1, epos-spos-1));
		int idx = stoi(tmpstr);
		spos = epos;
		epos = name.find_first_of( "_", spos+1);
		tmpstr = name.substr(spos+1, epos-spos-1);
		int conflictType = stoi(tmpstr);
		
		spos = name.find_last_of("_");
		double TTC = stoi(name.substr(spos+1));

		
		std::vector<double> colorVec;
		if (m_ColorCategory == 1)
		{
			if (TTC == 0)
			{
				colorVec = m_ConflictColorVecs[0];
			} else if (TTC > 0 && TTC <= 5)
			{
				colorVec = m_ConflictColorVecs[1];
			} else if (TTC > 5 && TTC <= 10)
			{
				colorVec = m_ConflictColorVecs[2];
			} else if (TTC > 10 && TTC <= 15)
			{
				colorVec = m_ConflictColorVecs[3];
			} else
			{
				colorVec = m_ConflictColorVecs[4];
			}
		} else if (m_ColorCategory == 0)
		{
			colorVec = m_ConflictColorVecs[conflictType];
		}
		osg::Vec4 shapeColor(colorVec[0], colorVec[1], colorVec[2], 1.0f);
		shapeDrawable->setColor(shapeColor);		
		shapeDrawable->dirtyDisplayList();
	}

	if (m_ColorCategory == 0)
	{
		RedrawBarChart(m_Roots[BARCHARTROOT]);
	}
}


void SSAMOSG::RedrawGrids()
{
	DrawGrids(m_Roots[GRIDROOT], m_Roots[BARCHARTROOT]);
	DrawContourMap(m_Roots[CONTOURROOT], m_Roots[HEATROOT]);
}

void SSAMOSG::RedrawContour()
{
	GenerateLevels();
	DrawContourMap(m_Roots[CONTOURROOT], m_Roots[HEATROOT]);
}

void SSAMOSG::RedrawHeat()
{
	for (unsigned int i = 0; i < m_Roots[HEATROOT]->getNumChildren(); ++i)
	{
		osg::Geode* geoNode = (osg::Geode*)(m_Roots[HEATROOT]->getChild(i));
		
		osg::ref_ptr<osg::Geometry> geom = (osg::Geometry*)(geoNode->getDrawable(0));
		
		std::string name(geoNode->getName());
		std::size_t spos = name.find_first_of("_");
				
		std::string tmpstr(name.substr(spos+1));
		int interval = stoi(tmpstr);

		geom->setColorArray(m_OSGIntervalColorVecs[interval], osg::Array::BIND_PER_VERTEX);
		
		geom->dirtyDisplayList();
	}
}

void SSAMOSG::RedrawMap()
{
	double m_Epsilon = 1e-6;
	if ((m_MapCoords[0] - m_MapCoords[2]) <= m_Epsilon
			&& (m_MapCoords[1] - m_MapCoords[3]) <= m_Epsilon)
	{
		return;
	}
	osg::Geode* geoNode = (osg::Geode*)(m_Roots[MAPROOT]->getChild(0));
	osg::Geometry* polyGeom = (osg::Geometry*)(geoNode->getDrawable(0));

	for (int i = 0; i < 4; ++i)
	{
		m_MapCoords[i] *= m_Feet2OSG * 1000.0;
	}

	double imageZ = 2.5;

	osg::Vec3 top_left(m_MapCoords[0], m_MapCoords[1], imageZ);
    osg::Vec3 bottom_left(m_MapCoords[0], m_MapCoords[3], imageZ);
    osg::Vec3 bottom_right(m_MapCoords[2], m_MapCoords[3], imageZ);
    osg::Vec3 top_right(m_MapCoords[2], m_MapCoords[1], imageZ);
    // anticlockwise ordering.
    osg::Vec3 myCoords[] =
    {
		top_left,
		bottom_left,
		bottom_right,
		top_right
    };

    int numCoords = sizeof(myCoords)/sizeof(osg::Vec3);

    // pass the created vertex array to the points geometry object.
    polyGeom->setVertexArray(new osg::Vec3Array(numCoords,myCoords));
	polyGeom->dirtyDisplayList();
}

void SSAMOSG::PreFrameUpdate()
{
	if (!m_ImgFileName.empty())
	{
		if (TakeScreenshot(m_ImgFileName))
			m_ImgFileName = "";
	}

	if (m_ResetFlag != 0)
	{
		if ((m_ResetFlag & 0x0004) == 0x04)
		{
			RedrawColor();
		}

		if ((m_ResetFlag & 0x0001) == 0x01)
		{
			RedrawScale();
		} 
		else if ((m_ResetFlag & 0x0008) == 0x08)
		{
			RedrawMap();
		}
		else if (m_ResetFlag == 0x0006)
		{
			RedrawShape();
			RedrawColor();
		}
		else if ((m_ResetFlag & 0x0002) == 0x02)
		{
			RedrawShape();
		}
		else if ((m_ResetFlag & 0x0010) == 0x10)
		{
			RedrawGrids();
		}
		else if ((m_ResetFlag & 0x0020) == 0x20) 
		{
			RedrawContour();
		}
		else if ((m_ResetFlag & 0x0040) == 0x40 )
		{
			RedrawHeat();
		}
		else if (m_ResetFlag == 0x0080)
		{
			RenderSceneGraph(m_ModelName, m_ConflictList, m_IsNewCase);
		}
		else if (m_ResetFlag == 0x0100)
		{
			DrawMap(m_Roots[MAPROOT]);
		}
		m_ResetFlag = 0x00;
	}
}

void SSAMOSG::PostFrameUpdate()
{
	if (!m_ImgFileName.empty())
	{
		if (TakeScreenshot(m_ImgFileName))
			m_ImgFileName = "";
	}

	if (m_pPickParam->GetIsReady())
	{
		ProcessPickParam();
		
	}
}

void SSAMOSG::ProcessPickParam()
{
	OSGMsgParam* param = NULL;

	const std::vector<std::vector<int> >& data = m_pPickParam->GetData();
	switch (m_MapType)
	{
	case CONFLICTS:
		if (m_pPickParam->GetFlag() == PickParam::LEFT)
		{
			for (std::vector<std::vector<int> >::const_iterator it = data.begin(); 
				it != data.end(); ++it)
			{
				if (it->empty() || it->front() != PickParam::CONFLICT) 
					continue;
				
				int idx = it->at(1);
				SP_Conflict refCP = FindConflictPoint(idx);
				if (refCP != NULL)
				{
					param = new OSGMsgParam();
					param->m_BL[0] = refCP->xMinPET - 1.0;
					param->m_BL[1] = refCP->yMinPET - 1.0;
					param->m_BL[2] = refCP->zMinPET - 1.0;

					param->m_TR[0] = refCP->xMinPET + 1.0;
					param->m_TR[1] = refCP->yMinPET + 1.0;
					param->m_TR[2] = refCP->zMinPET + 1.0;

					param->m_ConflictType = -1;
					param->m_FilterFlag = 0;
				}
				break; // only need to process one conflict point record
			}
		} else if (m_pPickParam->GetFlag() == PickParam::RIGHT)
		{
			std::vector<int> i, j;
			for (std::vector<std::vector<int> >::const_iterator it = data.begin(); 
				it != data.end(); ++it)
			{
				if (it->empty() || it->front() != PickParam::AREA) 
					continue;
				
				i.push_back(it->at(1));
				j.push_back(it->at(2));
			}
			if (!i.empty())
			{
				m_FilteredTileRoot = new osg::Group();
				param = new OSGMsgParam();
				GetFilteredTiles(i.size(), &(i[0]), &(j[0]), 
					param->m_BL, param->m_TR, param->m_FilteredIdx);
				param->m_ConflictType = -1;

				param->m_FilterFlag = 1;
			}
		}
		break;
	case BARCHART:
		for (std::vector<std::vector<int> >::const_iterator it = data.begin(); 
			it != data.end(); ++it)
		{
			if (it->empty() || it->front() != PickParam::BARCHART) 
				continue;

			int i = it->at(1);
			int j = it->at(2);
				
			param = new OSGMsgParam();
			if (!GetGridCoords(i, j, param->m_BL, param->m_TR))
			{
				delete param;
			} else
			{
				param->m_BL[2] = 0;
				param->m_ConflictType = it->at(3);
				param->m_FilterFlag = 0;
			}
			break; // only need to process one bar chart record
		}
		break;
	default:
		break;
	}
	
	if (param != NULL)
		PostMessage(m_hMapDlg, WM_OSGDATA, 0, (LPARAM)param);
		
	m_pPickParam->ResetIsReady();
}

bool SSAMOSG::TakeScreenshot(const std::string& filename)
{
	osg::ref_ptr<SaveImageDrawCallback> saveImageDrawCallback =
		dynamic_cast<SaveImageDrawCallback*>(m_Viewer->getCamera()->getPostDrawCallback());
    if(saveImageDrawCallback.get())
    {
        saveImageDrawCallback->SetFileName(filename);
        saveImageDrawCallback->SetIsSaveImage(true);
		return true;
    }
    else
    {
        return false;
    }
}

void SSAMOSG::SetRootMask()
{
	switch (m_MapType)
	{
	case CONFLICTS:
		m_Roots[MAPROOT]->setNodeMask(0x02);
		m_Roots[NETWORKROOT]->setNodeMask(0x02);
		m_Roots[CONFLICTROOT]->setNodeMask(0x02);
		m_Roots[GRIDROOT]->setNodeMask(0x01);
		m_Roots[BARCHARTROOT]->setNodeMask(0x01);
		m_Roots[CONTOURROOT]->setNodeMask(0x01);
		m_Roots[HEATROOT]->setNodeMask(0x01);
		break;
	case BARCHART:
		m_Roots[CONFLICTROOT]->setNodeMask(0x01);
		m_Roots[GRIDROOT]->setNodeMask(0x02);
		m_Roots[BARCHARTROOT]->setNodeMask(0x02);
		m_Roots[CONTOURROOT]->setNodeMask(0x01);
		m_Roots[HEATROOT]->setNodeMask(0x01);
		break;
	case CONTOUR:
		m_Roots[CONFLICTROOT]->setNodeMask(0x01);
		m_Roots[GRIDROOT]->setNodeMask(0x01);
		m_Roots[BARCHARTROOT]->setNodeMask(0x01);
		m_Roots[CONTOURROOT]->setNodeMask(0x02);
		m_Roots[HEATROOT]->setNodeMask(0x01);
		break;
	case HEAT:
		m_Roots[CONFLICTROOT]->setNodeMask(0x01);
		m_Roots[GRIDROOT]->setNodeMask(0x01);
		m_Roots[BARCHARTROOT]->setNodeMask(0x01);
		m_Roots[CONTOURROOT]->setNodeMask(0x02);
		m_Roots[HEATROOT]->setNodeMask(0x02);
		break;
	default:
		break;
	}
}

void SSAMOSG::SwitchMap(bool IsShown)
{
	if (IsShown)
		m_Roots[NETWORKROOT]->setNodeMask(0x02);
	else
		m_Roots[NETWORKROOT]->setNodeMask(0x01);
}

void SSAMOSG::ResetViewer() 
{
	m_Viewer->getCameraManipulator()->setNode(m_CameraMTF);
	m_Viewer->getCameraManipulator()->computeHomePosition();
	m_Viewer->home();
}

void SSAMOSG::DrawConflicts(osg::ref_ptr<osg::Group> ConflictRoot)
{	
	RemoveAllChildren(ConflictRoot);
	if (m_ConflictList.empty())
		return;
	m_SmallestZ  = INT_MAX;
	m_LargestZ = INT_MIN;
	int idx = 0;
	for (std::vector<SP_Conflict>::iterator itt = m_ConflictList.begin();
		itt != m_ConflictList.end(); ++itt)
	{
		SP_Conflict it = *itt;
		float zvalue = it->zMinPET * m_Feet2OSG;
		if (m_SmallestZ > zvalue)
			m_SmallestZ = zvalue;
		if (m_LargestZ < zvalue)
			m_LargestZ = zvalue;
		
		double px = it->xMinPET * m_Feet2OSG * 1000.0;
		double py = it->yMinPET * m_Feet2OSG * 1000.0;
		double pz = (it->zMinPET * m_Feet2OSG + m_ThickNessofRoad)  * 1000.0 +m_ConflictScale/2.0;
		
		osg::ref_ptr<osg::Shape> curShape = NULL;
		int shapeType = m_ShapeTypes[it->ConflictType - 1];
		switch (shapeType)
		{
		case 0:
			curShape = new osg::Cone(osg::Vec3(px, py, pz),m_ConflictScale/2.0, m_ConflictScale);
			break;
		case 1:
			curShape = new osg::Box(osg::Vec3(px, py, pz),m_ConflictScale);
			break;
		case 2:
			curShape = new osg::Sphere(osg::Vec3(px, py, pz),m_ConflictScale/2.0);
			break;
		}
		
		std::vector<double> colorVec;
		int TTC = it->TTC * 10+0.5;
		if (m_ColorCategory == 1)
		{
			
			if (TTC == 0)
			{
				colorVec = m_ConflictColorVecs[0];
			} else if (TTC > 0 && TTC <= 5)
			{
				colorVec = m_ConflictColorVecs[1];
			} else if (TTC > 5 && TTC <= 10)
			{
				colorVec = m_ConflictColorVecs[2];
			} else if (TTC > 10 && TTC <= 15)
			{
				colorVec = m_ConflictColorVecs[3];
			} else
			{
				colorVec = m_ConflictColorVecs[4];
			}
		} else if (m_ColorCategory == 0)
		{
			colorVec = m_ConflictColorVecs[it->ConflictType - 1];
		}
		osg::Vec4 shapeColor(colorVec[0], colorVec[1], colorVec[2], 1.0f);

		osg::ref_ptr<osg::ShapeDrawable> shapeDrawable = new osg::ShapeDrawable(curShape);
		shapeDrawable->setColor(shapeColor);
		shapeDrawable->setDataVariance(osg::Object::DYNAMIC);
		
		// add the points geometry to the geode.
		osg::ref_ptr<osg::Geode> Geode = new osg::Geode();
        Geode->addDrawable(shapeDrawable);
		std::string name("CP_" + std::to_string(idx++)+"_"+std::to_string(it->ConflictType - 1)+"_"+std::to_string(TTC));
		Geode->setName(name);
		ConflictRoot->addChild(Geode);
	}

	m_ExtremeZ = max(m_ExtremeZ, m_LargestZ);

}

void SSAMOSG::DrawMap(osg::ref_ptr<osg::Group> MapRoot)
{
	RemoveAllChildren(MapRoot);
	
	if (m_MapFile.empty())
		return;

	if ((m_MapCoords[0] - m_MapCoords[2]) <= m_Epsilon
		&& (m_MapCoords[1] - m_MapCoords[3]) <= m_Epsilon)
	{
		m_MapCoords[0] = m_SmallestX;
		m_MapCoords[1] = m_LargestY;
		m_MapCoords[2] = m_LargestX;
		m_MapCoords[3] = m_SmallestY;
	}
	
	double curMapCoords[4];
	for (int i = 0; i < 4; ++i)
	{
		curMapCoords[i] = m_MapCoords[i] * 1000.0;
	}

	double imageZ =  0;

	// we'll create a texture mapped quad to sit behind the Geometry
    osg::ref_ptr<osg::Image> image = osgDB::readImageFile(m_MapFile);
    if (!image) return;

    // create Geometry object to store all the vertices and lines primitive.
    osg::ref_ptr<osg::Geometry> polyGeom = new osg::Geometry();

	osg::Vec3 top_left(curMapCoords[0], curMapCoords[1], imageZ);
    osg::Vec3 bottom_left(curMapCoords[0], curMapCoords[3], imageZ);
    osg::Vec3 bottom_right(curMapCoords[2], curMapCoords[3], imageZ);
    osg::Vec3 top_right(curMapCoords[2], curMapCoords[1], imageZ);
    // anticlockwise ordering.
    osg::Vec3 myCoords[] =
    {
		top_left,
		bottom_left,
		bottom_right,
		top_right
    };

    int numCoords = sizeof(myCoords)/sizeof(osg::Vec3);

    // pass the created vertex array to the points geometry object.
    polyGeom->setVertexArray(new osg::Vec3Array(numCoords,myCoords));

    osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array;
    colors->push_back(osg::Vec4(1.0f,1.0f,1.0f,1.0f));
    polyGeom->setColorArray(colors, osg::Array::BIND_OVERALL);


    // set the normal in the same way color.
    osg::ref_ptr<osg::Vec3Array> normals = new osg::Vec3Array;
    normals->push_back(osg::Vec3(0.0f,-1.0f,0.0f));
    polyGeom->setNormalArray(normals, osg::Array::BIND_OVERALL);

    osg::Vec2 myTexCoords[] =
    {
        osg::Vec2(0,1),
        osg::Vec2(0,0),
        osg::Vec2(1,0),
        osg::Vec2(1,1)
    };

    int numTexCoords = sizeof(myTexCoords)/sizeof(osg::Vec2);

    // pass the created tex coord array to the points geometry object,
    // and use it to set texture unit 0.
    polyGeom->setTexCoordArray(0,new osg::Vec2Array(numTexCoords,myTexCoords));

    // well use indices and DrawElements to define the primitive this time.
    unsigned short myIndices[] =
    {
        0,
        1,
        3,
        2
    };

    int numIndices = sizeof(myIndices)/sizeof(unsigned short);

    polyGeom->addPrimitiveSet(new osg::DrawElementsUShort(osg::PrimitiveSet::TRIANGLE_STRIP,numIndices,myIndices));

    osg::ref_ptr<osg::StateSet> stateset = new osg::StateSet;

    osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
    texture->setImage(image);

    stateset->setTextureAttributeAndModes(0, texture,osg::StateAttribute::ON);

    polyGeom->setStateSet(stateset);
	polyGeom->setDataVariance(osg::Object::DYNAMIC);

    osg::ref_ptr<osg::Geode> geode = new osg::Geode();
	geode->addDrawable(polyGeom);

	MapRoot->addChild( geode);

}

bool SSAMOSG::GenerateGrids()
{
	m_NXGrids = ceil((m_LargestX - m_SmallestX)/m_GridSize); 
	m_NYGrids = ceil((m_LargestY - m_SmallestY)/m_GridSize);

	if (m_NXGrids <= 0 || m_NYGrids <= 0)
		return false;

	// generate Grids from left to right, bottom to top
	m_Grids.clear();
	m_Grids.resize(m_NXGrids, std::vector<GridInfo>(m_NYGrids, GridInfo()));
	for (int i = 0; i < m_NXGrids; ++i)
	{
		for (int j = 0; j < m_NYGrids; ++j)
		{
			GridInfo* pGrid = &(m_Grids[i][j]);
			pGrid->m_BL.x = m_SmallestX + m_GridSize * double(i);
			pGrid->m_BL.y = m_SmallestY + m_GridSize * double(j);
			pGrid->m_BL.z = m_ExtremeZ;

			pGrid->m_TR.x = m_SmallestX + m_GridSize * double(i+1);
			pGrid->m_TR.y = m_SmallestY + m_GridSize * double(j+1);
			pGrid->m_TR.z = m_ExtremeZ;

			pGrid->m_Center = (pGrid->m_BL + pGrid->m_TR) / 2.0;

			//pGrid->interval = 0;
			pGrid->m_TotalConflicts = 0;
			for (int k = 0; k < Conflict::NUM_CONFLICT_TYPES; ++k)
			{
				pGrid->m_NConflicts[k] = 0;
			}
		}
	}

	// external boundary points
	m_BBL = m_Grids[0][0].m_BL;
	m_BTR = m_Grids[m_NXGrids-1][m_NYGrids-1].m_TR;
	m_BBR = point(m_BTR.x, m_BBL.y, m_ExtremeZ);
	m_BTL = point(m_BBL.x, m_BTR.y, m_ExtremeZ);
	
	// grid points
	m_GBL = m_Grids[0][0].m_Center;
	m_GTR = m_Grids[m_NXGrids-1][m_NYGrids-1].m_Center;
	m_GBR = m_Grids[m_NXGrids-1][0].m_Center;
	m_GTL = m_Grids[0][m_NYGrids-1].m_Center;
	


	// count conflict points in grid
	for(std::vector<SP_Conflict>::iterator it = m_ConflictList.begin();
		it != m_ConflictList.end(); ++it)
	{
		SP_Conflict pConflict =*it;

		int i = floor((pConflict->xMinPET * m_Feet2OSG - m_SmallestX) / m_GridSize);
		if (i < 0) 
			i = 0;
		if (i >= m_NXGrids) 
			i = m_NXGrids - 1;

		int j = floor((pConflict->yMinPET * m_Feet2OSG - m_SmallestY) / m_GridSize);
		if (j < 0) 
			j = 0;
		if (j >= m_NYGrids) 
			j = m_NYGrids - 1;

		m_Grids[i][j].m_NConflicts[pConflict->ConflictType]++;
		m_Grids[i][j].m_TotalConflicts++;
	}

	// generate levels and contour data points
	GenerateLevels();

	return true;
}

void SSAMOSG::GenerateLevels()
{
	// generate levels and contour data points
	m_DataPoints.clear();
	m_MinIntervalVal = INT_MAX;
	m_MaxIntervalVal = 0;
	for (int i = 0; i < m_NXGrids; ++i)
	{
		for (int j = 0; j < m_NYGrids; ++j)
		{
			GridInfo* pGrid = &(m_Grids[i][j]);
			int n = pGrid->m_TotalConflicts;
			if (n < m_MinIntervalVal)
				m_MinIntervalVal = n;
			if (n > m_MaxIntervalVal)
				m_MaxIntervalVal = n;

			m_DataPoints.push_back(point(pGrid->m_Center.x, pGrid->m_Center.y, n));
		}
	}

	m_Levels.clear();
	double dVal = (m_MaxIntervalVal - m_MinIntervalVal) / double(m_NLevels+1);
	for (int i = m_NLevels; i >= 1; --i)
	{
		LevelInfo newLevel;
		newLevel.m_Value = dVal * double(i);
		
		int lidx = m_Levels.size();
		newLevel.m_ColorVec = new osg::Vec4Array;
		newLevel.m_ColorVec->push_back(m_TextColor);
		newLevel.m_ColorVec->push_back(m_TextColor);
		newLevel.m_ColorVec->push_back(m_TextColor);
		newLevel.m_ColorVec->push_back(m_TextColor);
		m_Levels.push_back(newLevel);	
	}
}

void SSAMOSG::RegenerateLevels()
{
	m_Levels.clear();
	double dVal = (m_MaxIntervalVal - m_MinIntervalVal) / double(m_NLevels+1);
	for (int i = m_NLevels; i >= 1; --i)
	{
		LevelInfo newLevel;
		newLevel.m_Value = dVal * double(i);
		
		int lidx = m_Levels.size();
		newLevel.m_ColorVec = new osg::Vec4Array;
		newLevel.m_ColorVec->push_back(m_TextColor);
		newLevel.m_ColorVec->push_back(m_TextColor);
		newLevel.m_ColorVec->push_back(m_TextColor);
		newLevel.m_ColorVec->push_back(m_TextColor);
		m_Levels.push_back(newLevel);	
	}

	for (int i = 0; i < m_NXGrids; ++i)
	{
		for (int j = 0; j < m_NYGrids; ++j)
		{
			GridInfo* pGrid = &(m_Grids[i][j]);
			// determine level
			int n = pGrid->m_TotalConflicts;
			int k = floor((double(n) - m_MinIntervalVal) / dVal);
			if (k < 0) k = 0;
			if (k > m_NLevels) k = m_NLevels;
			//pGrid->interval = m_NLevels - k;
		}
	}

	m_BBL.z = m_ExtremeZ;
	m_BBR.z = m_ExtremeZ;
	m_BTL.z = m_ExtremeZ;
	m_BTR.z = m_ExtremeZ;
}

void SSAMOSG::DrawGrids(osg::ref_ptr<osg::Group> GridRoot,
							osg::ref_ptr<osg::Group> BarChartRoot)
{
	if (!GenerateGrids())
		return;

	std::vector<std::vector<double> >& colorVecs = m_ConflictColorVecs;
	std::vector<osg::ref_ptr<osg::Vec4Array> > barColors;
	for (auto it = begin(colorVecs); it != end(colorVecs); ++it)
	{
		osg::Vec4 color(it->at(0), it->at(1), it->at(2), 1.0f);
		osg::ref_ptr<osg::Vec4Array> colorVec = new osg::Vec4Array;
		colorVec->push_back(color);
		colorVec->push_back(color);
		colorVec->push_back(color);
		colorVec->push_back(color);
		barColors.push_back(colorVec);
	}

	RemoveAllChildren(GridRoot);
	RemoveAllChildren(BarChartRoot);
	float charSize = m_GridSize / 10.0;
	double dVal = (m_MaxIntervalVal - m_MinIntervalVal) / double(m_NLevels+1);
	for (int i = 0; i < m_NXGrids; ++i)
	{
		for (int j = 0; j < m_NYGrids; ++j)
		{
			GridInfo* pGrid = &(m_Grids[i][j]);
			// determine level
			int n = pGrid->m_TotalConflicts;
			int k = floor((double(n) - m_MinIntervalVal) / dVal);
			if (k < 0) k = 0;
			if (k > m_NLevels) k = m_NLevels;
			//pGrid->interval = m_NLevels - k;

			// draw grid lines
			point TL(pGrid->m_BL.x, pGrid->m_TR.y, m_ExtremeZ);
			point BR(pGrid->m_TR.x, pGrid->m_BL.y, m_ExtremeZ);

			// draw left line
			DrawACustomWidthLine(TL, pGrid->m_BL, 
				m_LineWidth*10.0, m_HeightAbove, GridRoot, m_RedColorVec);

			// draw top line
			DrawACustomWidthLine(pGrid->m_TR, TL, 
				m_LineWidth*10.0, m_HeightAbove, GridRoot, m_RedColorVec);

			// draw right line only if at the end of row: compare index to row size
			if (i == m_NXGrids - 1)
				DrawACustomWidthLine(BR, pGrid->m_TR, 
					m_LineWidth*10.0, m_HeightAbove, GridRoot, m_RedColorVec);

			// draw bottom line only if at the beginning of column: compare index to column size
			if (j == 0)
				DrawACustomWidthLine(BR, pGrid->m_BL, 
					m_LineWidth*10.0, m_HeightAbove, GridRoot, m_RedColorVec);

			// draw bar charts on conflict types: 3
			// each base is square of 1/10 grid size, with the center in middle
			// height is the number of conflicts
			if (n > 0)
				DrawBarCharts(i, j, pGrid->m_NConflicts, Conflict::NUM_CONFLICT_TYPES, 
					pGrid->m_Center, charSize, BarChartRoot, barColors);

        }
	}
}

void SSAMOSG::GenerateContourLines()
{
	osg::ref_ptr<osg::Vec3Array> points = new osg::Vec3Array;
	for(std::vector<point>::iterator it = m_DataPoints.begin(); 
		it != m_DataPoints.end(); ++it) 
	{
		points->push_back(osg::Vec3(it->x, it->y, 0.0f));
	}
	osg::ref_ptr<osgUtil::DelaunayTriangulator> trig=new osgUtil::DelaunayTriangulator();
	trig->setInputPointArray(points);
	trig->triangulate();

	int origNumPoints = m_DataPoints.size();
	
	int nTrigs = 0;
	const osg::Vec3Array *vertices = NULL;
	const osg::PrimitiveSet* prset = NULL;

	osg::ref_ptr<osg::DrawElementsUInt> trigs = trig->getTriangles();
	if (trigs != NULL)
	{
		osg::ref_ptr<osg::Geometry> geom=new osg::Geometry;
		geom->addPrimitiveSet(trigs);
		vertices = dynamic_cast<const osg::Vec3Array *> (trig->getInputPointArray());
		nTrigs = trigs->getNumPrimitives();
		prset=geom->getPrimitiveSet(0);
	}

	int defI = -1;
	int defJ = -1;
	int c1 = 0;
	int c3 = 0;
	std::set<int> updatedIdx;
	int idx[3] = {-1, -1, -1};
	point p, pb;
	std::list<int> lineSeg;
	for (std::vector<LevelInfo>::iterator il = m_Levels.begin(); 
			il != m_Levels.end(); ++il)
	{
		for (int i = 0; i < nTrigs; ++i)
		{
			for (int j = 0; j < 3; ++j)
			{
				idx[j] = prset->index (i*3+j);
			}

			lineSeg.clear();
			GetPointOnEdge(idx[0], idx[1], *il, lineSeg);
			GetPointOnEdge(idx[1], idx[2], *il, lineSeg);
			GetPointOnEdge(idx[2], idx[0], *il, lineSeg);

			if (lineSeg.size() >=2 )
			{
				il->m_LineSegs.push_back(lineSeg);
			}
			
		}
		
		// concatenate line segments
		il->ConcatLines();
		int lscount = 0;
		// get point on outer boundary if the first or last point of the line is on inner boundary
		for (std::list<std::list<int> >::iterator ils = il->m_LineSegs.begin(); 
			ils != il->m_LineSegs.end(); ++ils)
		{
			if (ils->front() == ils->back())
			{
				continue;
			}

			int frontP = -1;
			std::list<int> tmpls;
			
			std::pair<int, int>& ce = il->m_PointToEdgeMap[ils->front()];
			int side1 = -1, side2 = -1;
			if (FindPointOnBoundary(il->m_CharPoints[ils->front()], ce.first,ce.second, p, side1))
			{
				frontP = il->InsertPoint(p);
			}
			
			ce = il->m_PointToEdgeMap[ils->back()];
			if (FindPointOnBoundary(il->m_CharPoints[ils->back()], ce.first,ce.second, p, side2))
			{
				tmpls.push_back(il->InsertPoint(p));
			}
			
			// side: 0: left; 1: right; 2: bottom ; 3: top
			if (side1 >=0 && side2 >= 0 && side1 != side2)
			{
				switch (side1)
				{
				case 0:
				{
					switch (side2)
					{
					case 1:
						tmpls.push_back(il->InsertPoint(m_BTR));
						tmpls.push_back(il->InsertPoint(m_BTL));
						break;
					case 2:
						tmpls.push_back(il->InsertPoint(m_BBL));
						break;
					case 3:
						tmpls.push_back(il->InsertPoint(m_BTL));
						break;
					}
				}
				break;
				case 1: // side: 0: left; 1: right; 2: bottom ; 3: top
				{
					switch (side2)
					{
					case 0:
						tmpls.push_back(il->InsertPoint(m_BTL));
						tmpls.push_back(il->InsertPoint(m_BTR));
						break;
					case 2:
						tmpls.push_back(il->InsertPoint(m_BBR));
						break;
					case 3:
						tmpls.push_back(il->InsertPoint(m_BTR));
						break;
					}
				}
				break;
				case 2: // side: 0: left; 1: right; 2: bottom ; 3: top
				{
					switch (side2)
					{
					case 0:
						tmpls.push_back(il->InsertPoint(m_BTL));
						break;
					case 1:
						tmpls.push_back(il->InsertPoint(m_BBR));
						break;
					case 3:
						tmpls.push_back(il->InsertPoint(m_BTR));
						tmpls.push_back(il->InsertPoint(m_BBR));
						break;
					}
				}
				break;
				case 3: // side: 0: left; 1: right; 2: bottom ; 3: top
				{
					switch (side2)
					{
					case 0:
						tmpls.push_back(il->InsertPoint(m_BTL));
						break;
					case 1:
						tmpls.push_back(il->InsertPoint(m_BTR));
						break;
					case 2:
						tmpls.push_back(il->InsertPoint(m_BBL));
						tmpls.push_back(il->InsertPoint(m_BTL));
						break;
					}
				}
				break;
				}
				
			}

			ils->push_front(frontP);
			ils->insert(ils->end(),tmpls.begin(), tmpls.end());
			
		}

		
		//////////////// apply Bezier Curve to smooth lines
		SmoothLines(*il);

	}
	
}

void SSAMOSG::DrawHeatMap(osg::ref_ptr<osg::Group> HeatRoot)
{
	RemoveAllChildren(HeatRoot);
	// add one more level as the lowest interval
	std::string name("HM_" + std::to_string(m_NLevels));
	DrawQuad(m_BBL, m_BBR, m_BTL, m_BTR, HeatRoot, m_OSGIntervalColorVecs[m_NLevels], name);

	// draw heat map
	for (int i = m_Levels.size() - 1; i >= 0; --i)
	{
		name = "HM_" + std::to_string(i);
		LevelInfo* pLevel = &(m_Levels[i]);
		std::vector<point>& cps = pLevel->m_CharPoints;
		std::list<std::list<int> >& lineSegs = pLevel->m_LineSegs;
		for (std::list<std::list<int> >::iterator it = lineSegs.begin();
			it != lineSegs.end(); ++it)
		{
			std::vector<point> points;
			for (std::list<int>::iterator pit = it->begin(); pit != it->end(); ++pit)
			{
				points.push_back(cps[*pit]);
				points.back().z += m_Delta * (m_NLevels - i);
			}
			
			if (it->size() == 3)
			{
				DrawTri(points[0], points[1], points[2], HeatRoot, m_OSGIntervalColorVecs[i], name);
			} else if (it->size() == 4)
			{
				DrawQuad(points[0], points[1], points[3], points[2], HeatRoot, m_OSGIntervalColorVecs[i], name);
			} else if (it->size() >= 5)
			{
				points.push_back(points.front());
				point p0 = CalcPolygonCentroid(points);
				p0.z = points.front().z;

				int nPoints = points.size();
				for (int ip = 0; ip < nPoints-1; ++ip)
				{
					point& p1 = points[ip];
					point& p2 = points[ip+1];
					DrawTri(p0, p1, p2, HeatRoot, m_OSGIntervalColorVecs[i], name);
				}
			}
		}
	}
}


bool SSAMOSG::HasChPoint(double lv, int i1, int i2, point& p)
{
	int dz = m_Epsilon;
	point& p1 = m_DataPoints[i1];
	point& p2 = m_DataPoints[i2];

	double v = (p1.z - lv) * (p2.z - lv);
	if ( v > 0)
	{
		return false;
	} else if (abs(v - 0.0) < m_Epsilon) // test v equal to 0
	{
		if (abs(p1.z - lv) < m_Epsilon)
			p1.z -= dz;
		else if (abs(p2.z - lv) < m_Epsilon)
			p2.z -= dz;

	} 
	double scale = (lv - p1.z) / (p2.z - p1.z);
	p.x = p1.x + (p2.x - p1.x)*scale;
	p.y = p1.y + (p2.y - p1.y)*scale;
	p.z = m_ExtremeZ;

	return true;
}

void SSAMOSG::DrawContourMap(osg::ref_ptr<osg::Group> ContourRoot,
								 osg::ref_ptr<osg::Group> HeatRoot)
{
	GenerateContourLines();

	RemoveAllChildren(ContourRoot);
	double zOffset = m_Delta * m_NLevels;
	for (std::vector<LevelInfo>::iterator il = m_Levels.begin(); 
			il != m_Levels.end(); ++il)
	{
		////////////////// draw contour lines
		for (std::list<std::list<int> >::iterator ils = il->m_LineSegs.begin(); 
			ils != il->m_LineSegs.end(); ++ils)
		{
			int nPoints = ils->size();
			int idx = 0;
			for (std::list<int>::iterator ip1 = ils->begin(), ip2 = std::next(ip1); 
				ip1 != ils->end() && ip2 != ils->end(); 
				++ip1, ++ip2)
			{
				point p1 = il->m_CharPoints[*ip1];
				p1.z += zOffset;
				point p2 = il->m_CharPoints[*ip2];
				p2.z += zOffset;
				DrawACustomWidthLine(p1, p2, 
					m_LineWidth * 10.0, m_HeightAbove, ContourRoot, il->m_ColorVec);
				idx++;

				if (idx == nPoints/2)
				{
					point pos = il->m_CharPoints[*ip1];
					pos.z += m_ThickNessofRoad + zOffset;
					WriteText(std::to_string(int(il->m_Value)),  
						pos, m_LineWidth * 100.0, m_TextColor, ContourRoot);
				}
			}
			
			// construct a closed polygon for open contour lines (which have points on outer boundary)
			// remove the last point from the closed contour lines
			if (!ils->empty() && ils->front() == ils->back())
			{
				ils->pop_back();
			}

		}
	}
	// add one more level as the lowest interval
	point TBBL = m_BBL;
	point TBBR = m_BBR;
	point TBTL = m_BTL;
	point TBTR = m_BTR;

	TBBL.z += zOffset;
	TBBR.z += zOffset;
	TBTL.z += zOffset;
	TBTR.z += zOffset;
	DrawACustomWidthLine(TBBL, TBBR, 
		m_LineWidth * 10.0, m_HeightAbove, ContourRoot, m_BlackColorVec);
	DrawACustomWidthLine(TBBR, TBTR, 
		m_LineWidth * 10.0, m_HeightAbove, ContourRoot, m_BlackColorVec);
	DrawACustomWidthLine(TBTR, TBTL, 
		m_LineWidth * 10.0, m_HeightAbove, ContourRoot, m_BlackColorVec);
	DrawACustomWidthLine(TBBL, TBTL, 
		m_LineWidth * 10.0, m_HeightAbove, ContourRoot, m_BlackColorVec);
	
	DrawHeatMap(HeatRoot);
}

void SSAMOSG::DrawBarCharts(int gi, int gj,
		int vals[], int nvals, 
		const point& center,
		float size, 
		osg::ref_ptr<osg::Group> BarChartRoot,
		std::vector<osg::ref_ptr<osg::Vec4Array> > barColors)
{
	std::vector<point> pos(nvals);
	pos[Conflict::REAR_END] = point(center.x - size / 2.0, center.y - size / 2.0, m_ExtremeZ);
	pos[Conflict::CROSSING] = pos[Conflict::REAR_END];
	pos[Conflict::CROSSING].x -= size;
	pos[Conflict::LANE_CHANGE] = pos[Conflict::REAR_END];
	pos[Conflict::LANE_CHANGE].x += size;

	// draw 
	for (int i = 1; i < nvals; ++i)
	{
		// name: BC_gi_gj_conflictType
		std::string name("BC_"+ std::to_string(gi) + "_" + std::to_string(gj) + "_" + std::to_string(i));
		// base points
		point& BL = pos[i];
		point BR(BL.x + size, BL.y, BL.z);
		point TL(BL.x, BL.y + size, BL.z);
		point TR(BR.x, TL.y, BL.z);
		
		// top points
		float height = vals[i] * m_Feet2OSG;
		point TBL = BL; TBL.z += height;
		point TBR = BR; TBR.z += height;
		point TTL = TL; TTL.z += height;
		point TTR = TR; TTR.z += height;

		if (vals[i] > 0)
		{
			// draw 4 faces
			DrawQuad(BL, BR, TBL, TBR, BarChartRoot, barColors[i-1], name);
			DrawQuad(BR, TR, TBR, TTR, BarChartRoot, barColors[i-1], name);
			DrawQuad(TR, TL, TTR, TTL, BarChartRoot, barColors[i-1], name);
			DrawQuad(TL, BL, TTL, TBL, BarChartRoot, barColors[i-1], name);
		}

		// draw top
		DrawQuad(TBL, TBR, TTL, TTR, BarChartRoot, barColors[i-1], name);

		point textPos(TBL);
		textPos.z += m_HeightAbove; 
		float charSize = m_GridSize / 10.0;
		WriteText(std::to_string(vals[i]),
			textPos,
			charSize,
			m_TextColor,
			BarChartRoot);
	}
}

void SSAMOSG::RedrawBarChart(osg::ref_ptr<osg::Group> BarChartRoot)
{
	std::vector<std::vector<double> >& colorVecs = m_ConflictColorVecs;
	std::vector<osg::ref_ptr<osg::Vec4Array> > barColors;
	for (auto it = begin(colorVecs); it != end(colorVecs); ++it)
	{
		osg::Vec4 color(it->at(0), it->at(1), it->at(2), 1.0f);
		osg::ref_ptr<osg::Vec4Array> colorVec = new osg::Vec4Array;
		colorVec->push_back(color);
		colorVec->push_back(color);
		colorVec->push_back(color);
		colorVec->push_back(color);
		barColors.push_back(colorVec);
	}

	RemoveAllChildren(BarChartRoot);
	float charSize = m_GridSize / 10.0;
	for (int i = 0; i < m_NXGrids; ++i)
	{
		for (int j = 0; j < m_NYGrids; ++j)
		{
			GridInfo* pGrid = &(m_Grids[i][j]);
			// determine level
			int n = pGrid->m_TotalConflicts;
			
			// Draw bar charts on conflict types: 3
			// each base is square of 1/10 grid size, with the center in middle
			// height is the number of conflicts
			if (n > 0)
				DrawBarCharts(i, j, pGrid->m_NConflicts, Conflict::NUM_CONFLICT_TYPES, 
					pGrid->m_Center, charSize, BarChartRoot, barColors);

        }
	}
}

bool SSAMOSG::FindPointOnBoundary(const point& p0, int i1, int i2, point& p, int& side)
{
	if (i1 < 0 || i2 < 0)
		return false;

	if ( i1 >= 0 && i1 < m_NYGrids 
		&& i2 >= 0 && i2 < m_NYGrids)// left
	{
		double scale = CalcP2PDist(p0, m_GBL) / CalcP2PDist(m_GBL, m_GTL);
		p = FindPointByScalar(m_BBL, m_BTL, scale);
		side = 0;
		return true;
	} else if ( i1 >= m_NYGrids*(m_NXGrids-1) && i2 >= m_NYGrids*(m_NXGrids-1) )// right
	{
		double scale = CalcP2PDist(p0, m_GBR) / CalcP2PDist(m_GBR, m_GTR);
		p = FindPointByScalar(m_BBR, m_BTR, scale);
		side = 1;
		return true;
	} else if ( (i1 % m_NYGrids == 0) && (i2 % m_NYGrids == 0)) // bottom
	{
		double scale = CalcP2PDist(p0, m_GBL) / CalcP2PDist(m_GBL, m_GBR);
		p = FindPointByScalar(m_BBL, m_BBR, scale);
		side = 2;
		return true;
	} else if ( ((i1+1) % m_NYGrids == 0) && ((i2+1) % m_NYGrids == 0)) // top
	{
		double scale = CalcP2PDist(p0, m_GTL) / CalcP2PDist(m_GTL, m_GTR);
		p = FindPointByScalar(m_BTL, m_BTR, scale);
		side = 3;
		return true;
	}
	
	return false;
}

void SSAMOSG::GetPointOnEdge(int i1, int i2, 
		LevelInfo& level, 
		std::list<int>& lineSeg)
{
	point p, pb;
	if (HasChPoint(level.m_Value, i1, i2, p))
	{
		int idx = level.InsertPoint(p, false, i1, i2, false);
		lineSeg.push_back(idx);
	}
}

int LevelInfo::InsertPoint(const point& p, bool isOnBoundary, 
		int i1 , int i2, bool isNew)
{
	if (!isNew)
	{
		for (int i = 0; i < m_CharPoints.size(); ++i)
		{
			if (m_CharPoints[i] == p)
			{
				return i;
			}
		}
	}

	m_CharPoints.push_back(p);
	m_PointOnBoundaryFlags.push_back(isOnBoundary);
	int pi1 = (i1 < i2) ? i1 : i2;
	int pi2 = (i1 < i2) ? i2 : i1;
	m_PointToEdgeMap.push_back(std::pair<int, int>(pi1, pi2));

	int idx = m_CharPoints.size() - 1;
	
	return idx;
}

void LevelInfo::ConcatLines()
{
	if (m_LineSegs.empty())
		return;

	std::vector<std::list<int> > l(m_LineSegs.begin(), m_LineSegs.end());
	for (int i = 0; i < l.size() - 1; ++i)
	{
		std::list<int>& ls1 = l[i];
		if (ls1.empty())
			continue;
		for (int j = i+1; j < l.size(); ++j)
		{
			std::list<int>& ls2 = l[j];
			if (ls2.empty())
				continue;

			if (ls1.front() == ls2.front())
			{
				ls2.pop_front();
				ls2.insert(ls2.begin(), ls1.rbegin(), ls1.rend());
				ls1.clear();
				break;
			} else if (ls1.back() == ls2.front())
			{
				ls2.pop_front();
				ls2.insert(ls2.begin(), ls1.begin(), ls1.end());
				ls1.clear();
				break;
			} else if (ls1.front() == ls2.back())
			{
				ls2.pop_back();
				ls2.insert(ls2.end(), ls1.begin(), ls1.end());
				ls1.clear();
				break;
			} else if (ls1.back() == ls2.back())
			{
				ls2.pop_back();
				ls2.insert(ls2.end(), ls1.rbegin(), ls1.rend());
				ls1.clear();
				break;
			} 
		}
	}

	m_LineSegs.clear();
	for (int i = 0; i < l.size(); ++i)
	{
		if (!l[i].empty())
			m_LineSegs.push_back(l[i]);
	}
}

void SSAMOSG::SmoothLines(LevelInfo& level)
{
	int defI = -1;
	int defJ = -1;
	double t = 0.75;
	int nSegs = 5;
	std::list<std::list<int> > newLSs;
	for (std::list<std::list<int> >::iterator ils = level.m_LineSegs.begin(); 
		ils != level.m_LineSegs.end(); ++ils)
	{
		int nP = ils->size();
		if (nP < 3)
			continue;
		int ipp = 0;

		std::list<int> newLS;
		point p01, p12;
		
		int remaining = nP;
		std::list<int>::iterator ip0 = ils->begin();
		std::list<int>::iterator ip1 = std::next(ip0);
		std::list<int>::iterator ip2 = std::next(ip1);
		while (ip2!= ils->end() && ipp < nP-2)
		{
			int idx0 = *ip0;
			int idx1 = *ip1;
			int idx2 = *ip2;
			point p0 = level.m_CharPoints[idx0];
			point p1 = level.m_CharPoints[idx1];
			point p2 = level.m_CharPoints[idx2];
			std::vector<point> controlPoints01;
			std::vector<point> controlPoints12;
			GenControlPoints(p0, p1, p2, t, controlPoints01, controlPoints12);

			// p0->p1
			if (ipp==0)
				newLS.push_back(idx0);
			if (level.m_PointOnBoundaryFlags[idx0] == false || level.m_PointOnBoundaryFlags[idx1] == false)
			{
				std::vector<point> ret;
				CalcCurveSegs(nSegs, controlPoints01, ret);
				for (std::vector<point>::iterator iret = ret.begin(); iret != ret.end(); ++iret)
				{
					newLS.push_back(level.InsertPoint(*iret, false));
				}
			}
			newLS.push_back(idx1);

			// last segment
			if (ipp == nP-3)
			{
				if (level.m_PointOnBoundaryFlags[idx1] == false || level.m_PointOnBoundaryFlags[idx2] == false)
				{
					std::vector<point> ret;
					CalcCurveSegs(nSegs, controlPoints12, ret);
					for (std::vector<point>::iterator iret = ret.begin(); iret != ret.end(); ++iret)
					{
						newLS.push_back(level.InsertPoint(*iret, false));
					}
				}
				newLS.push_back(idx2);
			}
				
			ipp++;
			if (ipp <= nP-2)
			{
				ip0 = ip1;
				ip1 = ip2;
				ip2 = std::next(ip1);
			}
				
		}
		newLSs.push_back( newLS);
	}
	level.m_LineSegs = newLSs;
}

void SSAMOSG::DrawTiles(osg::ref_ptr<osg::Group> TileRoot,
							int fidx[], osg::ref_ptr<osg::Group> FilteredTileRoot)
{
	double tileSize = 100.0 * m_Feet2OSG; // 5*5
	m_NXTiles = ceil((m_LargestX - m_SmallestX)/tileSize); 
	m_NYTiles = ceil((m_LargestY - m_SmallestY)/tileSize);

	if (m_NXTiles <= 0 || m_NYTiles <= 0)
		return;

	// generate Grids from left to right, bottom to top
	RemoveAllChildren(TileRoot);
	RemoveAllChildren(FilteredTileRoot);
	m_Tiles.clear();
	m_Tiles.resize(m_NXTiles, std::vector<TileInfo>(m_NYTiles, TileInfo()));
	for (int i = 0; i < m_NXTiles; ++i)
	{
		for (int j = 0; j < m_NYTiles; ++j)
		{
			TileInfo* pTile = &(m_Tiles[i][j]);
			pTile->m_BL.x = m_SmallestX + tileSize * double(i);
			pTile->m_BL.y = m_SmallestY + tileSize * double(j);
			pTile->m_BL.z = 0;

			pTile->m_TR.x = m_SmallestX + tileSize * double(i+1);
			pTile->m_TR.y = m_SmallestY + tileSize * double(j+1);
			pTile->m_TR.z = 0;

			point BR(pTile->m_TR.x, pTile->m_BL.y, pTile->m_BL.z);
			point TL(pTile->m_BL.x, pTile->m_TR.y, pTile->m_TR.z);

			std::string name("AR_" + std::to_string(i) + "_" + std::to_string(j));
			DrawQuad(pTile->m_BL, BR, TL, pTile->m_TR, TileRoot, m_RoadColorVec, name);

			int idx = i * m_NYTiles + j;
			WriteText( std::to_string(i) + "_" + std::to_string(j) + "_" + std::to_string(idx), 
				pTile->m_BL, tileSize/10.0, m_TextColor, TileRoot);

			//////////////// collect filtered tiles
			if (fidx[0] >= 0 
				&& i >= fidx[0] && i <= fidx[1]
				&& j >= fidx[2] && j <= fidx[3])
			{
				int idx = TileRoot->getNumChildren() - 1;
				osg::Geode* tileNode = (osg::Geode*)TileRoot->getChild(idx);
				FilteredTileRoot->addChild(tileNode);
			}
			
		}
	}
	TileRoot->setNodeMask(0x01);
}

void SSAMOSG::GetFilteredTiles(int n, int i[], int j[], 
						  double minBL[], double maxTR[], int fidx[])
{
	for (int k = 0; k < 2; ++k)
	{
		minBL[k] = FLT_MAX;
		maxTR[k] = FLT_MIN;
	}
	minBL[2] = 0;
	maxTR[2] = m_ExtremeZ;

	int mini = INT_MAX;
	int maxi = INT_MIN;
	int minj = INT_MAX;
	int maxj = INT_MIN;
	for (int k = 0; k < n; ++k)
	{
		if (i[k] >= 0 && i[k] < m_NXTiles 
			&& j[k] >= 0 && j[k] < m_NYTiles)
		{
			TileInfo* pTile = &(m_Tiles[i[k]][j[k]]);

			if (pTile->m_BL.x < minBL[0])
				minBL[0] = pTile->m_BL.x;

			if (pTile->m_BL.y < minBL[1])
				minBL[1] = pTile->m_BL.y;

			if (pTile->m_BL.z < minBL[2])
				minBL[2] = pTile->m_BL.z;

			if (pTile->m_TR.x > maxTR[0])
				maxTR[0] = pTile->m_TR.x;

			if (pTile->m_TR.y > maxTR[1])
				maxTR[1] = pTile->m_TR.y;

			if (pTile->m_TR.z > maxTR[2])
				maxTR[2] = pTile->m_TR.z;

			if (i[k] < mini)
				mini = i[k];

			if (i[k] > maxi)
				maxi = i[k];

			if (j[k] < minj)
				minj = j[k];

			if (j[k] > maxj)
				maxj = j[k];
		} 
	}
	// convert to FEET
	for (int k = 0; k < 3; ++k)
	{
		minBL[k] /= m_Feet2OSG;
		maxTR[k] /= m_Feet2OSG;
	}

	// filtered tile indexes
	fidx[0] = mini;
	fidx[1] = maxi;
	fidx[2] = minj;
	fidx[3] = maxj;
}

void SSAMOSG::GenControlPoints(point p0, point p1, point p2, double t,
		std::vector<point>& controlPoints01, std::vector<point>& controlPoints12)
{
	double d01 = CalcP2PDist(p0, p1);
	double d12 = CalcP2PDist(p1, p2);
	double total = d01 + d12;
	double scale01 = t * d01 / total;   
    double scale12 = t * d12 / total;   

	point p01 = p1 - (p2 - p0) * scale01;
	point p12 = p1 + (p2 - p0) * scale12;
    
	point c011 = (p0 + p01) / 2.0;
	point c012 = (p1 + p01) / 2.0;
	point c121 = (p1 + p12) / 2.0;
	point c122 = (p2 + p12) / 2.0;

	controlPoints01.push_back(p0);
	controlPoints01.push_back(c011);
	controlPoints01.push_back(c012);
	controlPoints01.push_back(p1);

	controlPoints12.push_back(p1);
	controlPoints12.push_back(c121);
	controlPoints12.push_back(c122);
	controlPoints12.push_back(p2);
}

void SSAMOSG::CalcCurveSegs(int nSegments, std::vector<point>& controlPoints, 
		std::vector<point>& ret)
{
	ret.clear();
	float d = 1.0 / float(nSegments);
    point p;
	for (int i = 1; i<nSegments; ++i)
    {
      float t = d * float(i);
      float t2 = t * t;
      float ct = 1.0 - t; 
      float ct2 = ct * ct;
      p = controlPoints[0] * ct2 * ct +
          controlPoints[1] * 3.0 * t * ct2 +
          controlPoints[2] * 3.0 * t2 * ct +
          controlPoints[3] * t2 * t;
	  p.z = m_ExtremeZ;
	  ret.push_back(p);
    }
}

void SSAMOSG::RemoveAllChildren(osg::ref_ptr<osg::Group> root)
{
	int numChildren = root->getNumChildren();
	for (int i = 0; i < numChildren; ++i)
	{
		osg::Node* node = root->getChild(0);
		root->removeChild(node);
	}
}


void SSAMOSG::CreateColorVectors()
{
	float RoadColorValue =  0.7451;
	float RedColorValue = 1.00f;

	// Define colors for different things and put them into each color vector in OSG
	osg::Vec4 RoadColor(RoadColorValue, RoadColorValue, RoadColorValue, 1.0f);
	osg::Vec4 RedLineColor(RedColorValue, 0.0f, 0.0f, 1.0f);
	m_TextColor = osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f);
	
	m_RoadColorVec = new osg::Vec4Array;
	m_RoadColorVec->push_back(RoadColor);
	m_RoadColorVec->push_back(RoadColor);
	m_RoadColorVec->push_back(RoadColor);
	m_RoadColorVec->push_back(RoadColor);

	m_RedColorVec = new osg::Vec4Array;
	m_RedColorVec->push_back(RedLineColor);
	m_RedColorVec->push_back(RedLineColor);
	m_RedColorVec->push_back(RedLineColor);
	m_RedColorVec->push_back(RedLineColor);

	m_BlackColorVec = new osg::Vec4Array;
	m_BlackColorVec->push_back(m_TextColor);
	m_BlackColorVec->push_back(m_TextColor);
	m_BlackColorVec->push_back(m_TextColor);
	m_BlackColorVec->push_back(m_TextColor);
}

point SSAMOSG::CalcPerpOffset(double x1, double y1,double x2,double y2,double dist)
{
	double addX, addY, slope;
	point offset;
	if(x1 == x2 )
	{
		addX = dist;
		addY = 0;
	}
	else if (y1 == y2)
	{
		addX = 0;
		addY = dist;
	}
	else
	{
		slope = (y2-y1)/(x2-x1);
		addX = dist*sqrt(abs(1.0/(1.0+1.0/(slope*slope))));
		addY = abs(-1/slope*addX);
	}
	
	if(y2 > y1)
		addX *= -1;
	if(x2 < x1)
		addY *= -1;

	offset.x = addX;
	offset.y = addY;
	return offset;
}

void SSAMOSG::DrawACustomWidthLine(point startPoint,
							  point endPoint,
							  double width,
							  double heightAbove,
							  osg::Group* root,
							  osg::Vec4Array* colors)
{
	point onelineoffset = CalcPerpOffset(startPoint.x, startPoint.y, endPoint.x, endPoint.y, width)  * 1000.0;
	startPoint = startPoint * 1000.0;
	endPoint = endPoint * 1000.0;
	heightAbove = heightAbove * 1000.0;

	osg::ref_ptr<osg::Geometry> LineGeometry = new osg::Geometry();

	osg::ref_ptr<osg::Vec3Array> LineVertices = new osg::Vec3Array();
	LineVertices->push_back(osg::Vec3(startPoint.x + (onelineoffset.x), startPoint.y + (onelineoffset.y ), startPoint.z+heightAbove));
	LineVertices->push_back(osg::Vec3(startPoint.x - (onelineoffset.x ), startPoint.y - (onelineoffset.y ), startPoint.z+heightAbove));
	LineVertices->push_back(osg::Vec3(endPoint.x - (onelineoffset.x ), endPoint.y - (onelineoffset.y ), endPoint.z+heightAbove));
	LineVertices->push_back(osg::Vec3(endPoint.x + (onelineoffset.x ), endPoint.y + (onelineoffset.y ), endPoint.z+heightAbove));
	LineGeometry->setVertexArray(LineVertices);

	osg::ref_ptr<osg::DrawElementsUInt> LineBase = new osg::DrawElementsUInt(osg::PrimitiveSet::POLYGON, 0);
	LineBase->push_back(0);
	LineBase->push_back(1);
	LineBase->push_back(2);
	LineBase->push_back(3);
	LineGeometry->addPrimitiveSet(LineBase);

	LineGeometry->setColorArray(colors);
	LineGeometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

	osg::ref_ptr<osg::Geode> LineGeode = new osg::Geode();
	LineGeode->addDrawable(LineGeometry);
	LineGeode->setNodeMask(0x2);
	root->addChild(LineGeode);
}

void SSAMOSG::WriteText(const std::string& text, 
		point position, 
		float characterSize,
		const osg::Vec4& color,
		osg::ref_ptr<osg::Group> textRoot)
{
	position = position * 1000.0;
	characterSize *= 1000.0;

	osg::Vec3 pos(position.x, position.y, position.z);

	osg::ref_ptr<osgText::Text3D> text3D = new osgText::Text3D;
    text3D->setFont(m_Font.get());
    text3D->setCharacterSize(characterSize);
	text3D->setDrawMode(osgText::Text3D::TEXT );
    text3D->setAxisAlignment(osgText::Text3D::XY_PLANE);
    text3D->setText(text);
	text3D->setColor(color);
	text3D->setPosition(pos);
	text3D->setDataVariance(osg::Object::DYNAMIC);

    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    geode->addDrawable(text3D);
	geode->setNodeMask(0x02);
    textRoot->addChild(geode);
}

void SSAMOSG::DrawQuad(point BL, point BR, point TL, point TR, 
							osg::Group* root, osg::Vec4Array* colors, 
							const std::string& name)
{
	BL = BL * 1000.0;
	BR = BR * 1000.0;
	TL = TL * 1000.0;
	TR = TR * 1000.0;

	osg::Vec3 top_left(TL.x, TL.y, TL.z);
    osg::Vec3 bottom_left(BL.x, BL.y, BL.z);
    osg::Vec3 bottom_right(BR.x, BR.y, BR.z);
    osg::Vec3 top_right(TR.x, TR.y, TR.z);
	osg::Vec3 myCoords[] =
    {
		top_left,
		bottom_left,
		bottom_right,
		top_right
    };
    int numCoords = sizeof(myCoords)/sizeof(osg::Vec3);

	osg::ref_ptr<osg::Geometry> polyGeom = new osg::Geometry();
    polyGeom->setVertexArray(new osg::Vec3Array(numCoords,myCoords));
    polyGeom->setColorArray(colors, osg::Array::BIND_OVERALL);
	osg::ref_ptr<osg::Vec3Array> normals = new osg::Vec3Array;
    normals->push_back(osg::Vec3(0.0f,-1.0f,0.0f));
    polyGeom->setNormalArray(normals, osg::Array::BIND_OVERALL);
	polyGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS,0,numCoords));
	polyGeom->setDataVariance(osg::Object::DYNAMIC);
	
	osg::ref_ptr<osg::Geode> geode = new osg::Geode();
	geode->addDrawable(polyGeom);
	geode->setName(name);
	geode->setNodeMask(0x2);
	root->addChild(geode);
}

void SSAMOSG::DrawTri(point p1, point p2, point p3, osg::Group* root, osg::Vec4Array* colors,
						  const std::string& name)
{
	p1 = p1 * 1000.0;
	p2 = p2 * 1000.0;
	p3 = p3 * 1000.0;

	osg::Vec3 point1 = osg::Vec3(p1.x,p1.y,p1.z);
	osg::Vec3 point2 = osg::Vec3(p2.x,p2.y,p2.z);
	osg::Vec3 point3 = osg::Vec3(p3.x,p3.y,p3.z);
	osg::Vec3 myCoords[] =
    {
		point1,
		point2,
		point3
    };
	int numCoords = sizeof(myCoords)/sizeof(osg::Vec3);

	osg::ref_ptr<osg::Geometry> polyGeom = new osg::Geometry();
    polyGeom->setVertexArray(new osg::Vec3Array(numCoords,myCoords));
	polyGeom->setColorArray(colors, osg::Array::BIND_OVERALL);
	osg::ref_ptr<osg::Vec3Array> normals = new osg::Vec3Array;
    normals->push_back(osg::Vec3(0.0f,-1.0f,0.0f));
    polyGeom->setNormalArray(normals, osg::Array::BIND_OVERALL);
	polyGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLES,0,numCoords));
	polyGeom->setDataVariance(osg::Object::DYNAMIC);
	
	osg::ref_ptr<osg::Geode> geode = new osg::Geode();
	geode->addDrawable(polyGeom);
	geode->setName(name);
	geode->setNodeMask(0x2);
	root->addChild(geode);
}

point SSAMOSG::CalcPolygonCentroid(const std::vector<point>& sps)
{
	point p;
    double area = 0; 
    double x = 0;
    double y = 0;
	for (int i = 0; i < (int)sps.size() - 1; i++) 
	{
        double tmp = sps[i].x * sps[i + 1].y - sps[i + 1].x * sps[i].y;
        area += tmp; 
        x += (sps[i].x + sps[i + 1].x) * tmp;
        y += (sps[i].y + sps[i + 1].y) * tmp;
    }
	area /= 2.0;
	if (abs(area) >  1e-09)
	{
		p.x = x / (6.0 * area);
		p.y = y / (6.0 * area);
	} else
	{
        double areaSum = 0;
        for (int i = 0; i < (int)sps.size() - 1; i++) 
		{
			double subArea = CalcP2PDist(sps[i],sps[i + 1]);
            x += (sps[i].x + sps[i + 1].x) / 2.0 * subArea ;
            y += (sps[i].y + sps[i + 1].y) / 2.0 * subArea;
            areaSum += subArea;
        }
        if (abs(areaSum) < 1e-09) {
            return sps[0];
        }
		p.x = x / areaSum;
		p.y = y / areaSum;
	}
	return p;
}

point SSAMOSG::FindPointByScalar(point p1, point p2, double scalar)
{
	point p ;
	p.x = p1.x + (p2.x - p1.x) * scalar ;
	p.y = p1.y + (p2.y - p1.y) * scalar;
	p.z = p1.z + (p2.z - p1.z) * scalar;
	return p;
}

CRenderingThread::CRenderingThread( SSAMOSG* ptr )
	: OpenThreads::Thread()
	, _ptr(ptr)
	, _done(false)
{
}

CRenderingThread::~CRenderingThread()
{
    _done = true;
    while( isRunning() )
        OpenThreads::Thread::YieldCurrentThread();
}

void CRenderingThread::run()
{
    if ( !_ptr )
    {
        _done = true;
        return;
    }

    osgViewer::Viewer* viewer = _ptr->GetViewer();
    do
    {
        _ptr->PreFrameUpdate();
        viewer->frame();
        _ptr->PostFrameUpdate();
    } while ( !testCancel() && !viewer->done() && !_done );
}

bool PickHandler::handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& aa)
{
	if(ea.getEventType()==osgGA::GUIEventAdapter::RELEASE)
	{
		if (ea.getButton()==osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON )
		{
			m_pParam->SetFlag(PickParam::LEFT);
		} else if (ea.getButton()==osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON)
		{
			m_pParam->SetFlag(PickParam::RIGHT);
		}

		osgViewer::View* view = dynamic_cast<osgViewer::View*>(&aa);
		if (view) pick(view,ea);
		
		return false;
	}

	return false;
	
}

void PickHandler::pick(osgViewer::View* view, const osgGA::GUIEventAdapter& ea)
{
    osgUtil::LineSegmentIntersector::Intersections intersections;
	if (view->computeIntersections(ea,intersections))
    {
        for(osgUtil::LineSegmentIntersector::Intersections::iterator hitr = intersections.begin();
            hitr != intersections.end();
            ++hitr)
        {
            std::ostringstream os;
            if (!hitr->nodePath.empty() && !(hitr->nodePath.back()->getName().empty()))
            {
                // the geodes are identified by name.
				std::string tmpStr = hitr->nodePath.back()->getName();
				if ( m_pParam->GetFlag() == PickParam::LEFT 
					&& (tmpStr.substr(0, 2)=="CP" || tmpStr.substr(0, 2)=="BC") )
					m_pParam->SetName( hitr->nodePath.back()->getName());
				else if (m_pParam->GetFlag() == PickParam::RIGHT && tmpStr.substr(0, 2)=="AR")
					m_pParam->SetName( hitr->nodePath.back()->getName());
				
            }
        }
    } else
	{
		m_pParam->ResetIsReady();
	}
}

void PickParam::ParseName(const std::string& name)
{
	std::vector<int> subdata;
	std::istringstream ss(name);
	std::string field;
	while (std::getline(ss, field, '_'))
	{
		if (field == "CP")
		{
			subdata.push_back(CONFLICT);
		} else if (field == "BC")
		{
			subdata.push_back(BARCHART);
		} else if (field == "AR")
		{
			subdata.push_back(AREA);
		} else
		{
			subdata.push_back(stoi(field));
		}
	}
	m_Data.push_back(subdata);
}
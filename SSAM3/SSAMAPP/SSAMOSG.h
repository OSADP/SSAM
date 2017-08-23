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
#pragma once
#define _USE_MATH_DEFINES
#include<vector>
#include <fstream>
#include <cmath>
#include "windows.h"
#include "Conflict.h"
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/api/win32/GraphicsWindowWin32>
#include <osgGA/TrackballManipulator>
#include <osgGA/KeySwitchMatrixManipulator>
#include <osg/ShapeDrawable>
#include <osg/matrixtransform>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgText/Font>
#include <osgText/Text3D>


#define M_PI       3.14159265358979323846

/** User-defined message for passing parameters selected from OSG viewer
*/
#define WM_OSGDATA (WM_USER + 100)

/** OSGMsgParam organizes parameters selected from OSG viewer and passed by WM_OSGDATA
*/
struct OSGMsgParam
{
	double m_BL[3]; /*!< Bottom left point of selected area */ 
	double m_TR[3]; /*!< Top right point point of selected area */ 
	int m_ConflictType; /*!< Selected conflict type */
	int m_FilterFlag; /*!< Flag indicates whether to filter out selected conflicts */
	int m_FilteredIdx[4]; /*!< An array of indexes identifying filtered area: mini, maxi, minj, maxj  */
};

/** Point class defines operators 
*/
class point
{
public:
	double x;
	double y;
	double z;
	double epsilon;

	point(double ix = 0.0, double iy = 0.0, double iz = 0.0)
		: x (ix)
		, y (iy)
		, z (iz)
		, epsilon(0.000001)
	{}

	~point() {}

	point operator+(const point& rhs)
	{
		point newPoint;
		newPoint.x = this->x + rhs.x;
		newPoint.y = this->y + rhs.y;
		newPoint.z = this->z + rhs.z;
		return newPoint;
	}

	point operator+(double addition)
	{
		point newPoint;
		newPoint.x = this->x + addition;
		newPoint.y = this->y + addition;
		newPoint.z = this->z + addition;
		return newPoint;
	}

	point& operator+=(const point& rhs)
	{
		this->x += rhs.x;
		this->y += rhs.y;
		this->z += rhs.z;
		return *this;
	}
	
	point operator-(const point& rhs)
	{
		point newPoint;
		newPoint.x = this->x - rhs.x;
		newPoint.y = this->y - rhs.y;
		newPoint.z = this->z - rhs.z;
		return newPoint;
	}

	point& operator-=(const point& rhs)
	{
		this->x -= rhs.x;
		this->y -= rhs.y;
		this->z -= rhs.z;
		return *this;
	}

	point operator*(double scale)
	{
		point newPoint;
		newPoint.x = this->x * scale;
		newPoint.y = this->y * scale;
		newPoint.z = this->z * scale;
		return newPoint;
	}

	point operator/(double scale)
	{
		point newPoint;
		if (scale != 0.0)
		{
			newPoint.x = this->x / scale;
			newPoint.y = this->y / scale;
			newPoint.z = this->z / scale;
		}
		return newPoint;
	}

	bool operator==(const point& rhs)
	{
		return (abs(this->x - rhs.x) <= epsilon
			&& abs(this->y - rhs.y) <= epsilon
			&& abs(this->z - rhs.z) <= epsilon);
	}

	bool operator!=(const point& rhs)
	{
		return (abs(this->x - rhs.x) > epsilon
			|| abs(this->y - rhs.y) > epsilon
			|| abs(this->z - rhs.z) > epsilon);
	}

};

/** GridInfo organizes parameters of one grid cell
*/
struct GridInfo
{
	point m_BL; /*!< Bottom left point of grid cell */ 
	point m_TR; /*!< Top right point of grid cell */ 
	point m_Center; /*!< Center point of grid cell */
	//int m_Interval;
	int m_TotalConflicts; /*!< Total number of conflicts in grid cell */
	/*!< Array of conflict numbers of each type in grid cell */
	int m_NConflicts[Conflict::NUM_CONFLICT_TYPES];
};

/** TileInfo organizes parameters of one tile
* used for selecting filtering area
*/
struct TileInfo
{
	point m_BL; /*!< Bottom left point of grid cell */
	point m_TR; /*!< Top right point point of grid cell */ 
};

/** LevelInfo organizes parameters of one contour level
*/
struct LevelInfo
{
	double m_Value; /*!< Value of contour level */
	osg::ref_ptr<osg::Vec4Array> m_ColorVec; /*!< Color used to draw contour level */
	std::vector<point> m_CharPoints; /*!< Characteristic points of contour level */
	std::list<std::list<int> > m_LineSegs; /*!< Line segments to form contour level */
	/*!< Vector of triangle edges that the characteristic points come from
	* same index as m_CharPoints
	*/
	std::vector<std::pair<int, int> > m_PointToEdgeMap;
	/*!< Vector of flags indicate whether a characteristic point is on network boundary
	* same index as m_CharPoints
	*/
	std::vector<bool> m_PointOnBoundaryFlags;
	
	/** Insert a characteristic point and return its index
	* @param p characteristic point to insert
	* @param isOnBoundary flag indicates whether point is on network boundary
	* @param i1 first point of edge of a triangle; -1 means it is not from triangle
	* @param i2 second point of edge of a triangle; -1 means it is not from triangle
	* @param isNew flag indicates whether to treat the inserting point as a new point
	* @return the idex of the characteristic point
	*/
	int InsertPoint(const point& p, bool isOnBoundary = true, 
		int i1 = -1, int i2 = -1, bool isNew = true);

	/** Concatenate line segments to form contour level line
	*/
	void ConcatLines();
};

/** PickParam manages parameters picked from OSG viewer 
*/
class PickParam
{
public:
	/** PICK_FLAG enum defines the pressed mouse button 
	*/
	enum PICK_FLAG 
	{
		LEFT,
		RIGHT
	};

	/** DATA_TYPE enum defines object type of the picked parameters 
	*/
	enum DATA_TYPE
	{
		CONFLICT,
		BARCHART,
		AREA
	};

	PickParam()
		: m_IsReady(false)
		, m_Flag(LEFT)
		, m_DataType(CONFLICT)
	{	}

	~PickParam(){}

	/** Reset flag indicating whether data is ready. 
	*/
	void ResetIsReady() 
	{
		m_IsReady = false; 
		m_Data.clear();
	}

	/** Get flag indicating whether data is ready. 
	* if picking conflict point or bar chart, only need one set of data
	* if picking area, need at least two sets of data
	*/
	bool GetIsReady() 
	{
		return m_IsReady && ((m_Flag == LEFT) ? 
			(!m_Data.empty()) 
			: (m_Data.size() >= 2) );
	}

	/** Set object name and data ready flag.
	* @param name object name
	*/
	void SetName(const std::string& name) 
	{ 
		ParseName(name);
		m_IsReady = true;
	}

	void SetFlag(int fl) { m_Flag = static_cast<PICK_FLAG>(fl);}
	PICK_FLAG GetFlag() {return m_Flag;}
	const std::vector<std::vector<int> >& GetData() {return m_Data;}
private:
	bool m_IsReady; /*!< Flag indicates whether data is ready. */
	PICK_FLAG m_Flag; /*!< The pressed mouse button. */
	DATA_TYPE m_DataType; /*!< The picked object type. */
	/*!< Parameters of the picked object type. 
	* first element of each vector is m_Flag
	*/
	std::vector<std::vector<int> > m_Data; 

	/** Parse object name and convert into parameters. 
	* @param name object name
	*/
	void ParseName(const std::string& name);
};

/** Smart pointer type to PickParam class.
*/
typedef std::shared_ptr<PickParam> SP_PickParam;

/** PickHandler class handles events with a pick in OSG viewer
*/
class PickHandler : public osgGA::GUIEventHandler {
public:

    PickHandler(SP_PickParam pParam):
		m_pParam(pParam) {}

    ~PickHandler() {}

	/** Implement virtual function. Handle events, return true if handled, false otherwise. 
	*/
    bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& aa);

	/** Generate parameters from picked objects in OSG viewer 
	*/
    void pick(osgViewer::View* view, const osgGA::GUIEventAdapter& ea);
	
protected:
    SP_PickParam m_pParam ; /*!< parameters picked from OSG viewer */
};

/** SSAMOSG renders conflict maps in OSG viewer 
*/
class SSAMOSG
{
public:
	/** COLOR_CATEGORY enum defines color category of conflict points 
	*/
	enum COLOR_CATEGORY
	{
		TYPE = 0, /*!< conflict type  */
		TTC = 1 /*!< conflict TTC  */
	};
	const static int NUM_TTC_LEVELS  = 5; /*!< The number of TTC levels */
	
	/** MAP_TYPE enum defines map types to render 
	*/
	enum MAP_TYPE
	{
		CONFLICTS,
		BARCHART,
		CONTOUR,
		HEAT
	};
	const static int NUM_MAP_TYPES = 4; /*!< The number of map types */

	/** ROOT_TYPE enum defines group types for organizing OSG models 
	*/
	enum ROOT_TYPE
	{
		NETWORKROOT, /*!< Group of roadway network models */
		CONFLICTROOT, /*!< Group of conflict point models */
		TILEROOT, /*!< Group of underneath tile models for filtering by clicking map */
		MAPROOT, /*!< Group of map background image */
		GRIDROOT, /*!< Group of grid cell models */
		BARCHARTROOT, /*!< Group of bar chart models */
		CONTOURROOT, /*!< Group of contour map models */
		HEATROOT, /*!< Group of heat map models */
	};
	const static int NUM_OF_ROOT_TYPES = 8; /*!< The number of OSG group types */

	/** Constructor creates a SSAMOSG object.
	* @param hWnd the handle to the window for creating OSG viewer
	* @param hMapDlg the handle to Dlg_Map window for configuring map
	*/
	SSAMOSG(HWND hWnd, HWND hMapDlg);
	~SSAMOSG(void);

	/** Initialize OSG viewer.
	*/
	void InitOSG();

	/** Perform operations before OSG viewer updates frame.
	*/
	void PreFrameUpdate(void);

	/** Perform operations after OSG viewer updates frame.
	*/
    void PostFrameUpdate(void);

	/** Reset OSG viewer to initial status.
	*/
	void ResetViewer(); 

	/** Add map background image.
	* @param f file name of the map background image.
	*/
	void EditMap(const std::string& f);

	/** Switch roadway map model on or off.
	* @param IsShown flag to indicate whether to show roadway map model.
	*/
	void SwitchMap(bool IsShown);

	// get() /set() methods
	osgViewer::Viewer* GetViewer() { return m_Viewer; }
	const std::string& GetImageFileName() const {return m_ImgFileName;}	
	void SetSceneGraph(std::string filename, const std::list<SP_Conflict>& conflictList, bool isNewCase);
	void SetImageFileName(const std::string& f) {m_ImgFileName = f;}
	void SetMapType(int mt) { m_MapType = mt;}
	void SetMapFile(const std::string& mf ) {m_MapFile = mf;}
	void SetMapCoords(double c[]) 
	{
		for (int i = 0; i < 4; ++i)
		{
			m_MapCoords[i] = c[i] * m_Feet2OSG;
		}
	}
	void SetConflictScale(double cs) {m_ConflictScale = cs;}
	void SetShapeTypes(const std::vector<int>& STs){m_ShapeTypes = STs;}
	void SetColorCategory(int cc) {m_ColorCategory = cc;}
	void SetColorVecs(const std::vector<std::vector<double> >& CCVs) 
	{
		m_ConflictColorVecs = CCVs;	
	}
	void SetGridSize(double gs) { m_GridSize = gs * m_Feet2OSG;}
	void SetNLevels(int nl) { m_NLevels = nl;}
	void SetIntervalColorVecs(const std::vector<std::vector<double> >& ICVs)
	{
		m_IntervalColorVecs = ICVs;
		m_OSGIntervalColorVecs.clear();
		for (std::vector<std::vector<double> >::iterator it = m_IntervalColorVecs.begin();
			it != m_IntervalColorVecs.end(); ++it)
		{
			osg::Vec4 color(it->at(0), it->at(1), it->at(2), 1.0f);
			osg::ref_ptr<osg::Vec4Array> colorVec = new osg::Vec4Array;
			colorVec->push_back(color);
			colorVec->push_back(color);
			colorVec->push_back(color);
			colorVec->push_back(color);
			m_OSGIntervalColorVecs.push_back(colorVec);	
		}
	}
	void SetMapBoundaries(int c[]) 
	{
		m_SmallestX = double(c[0]) * m_Feet2OSG;
		m_SmallestY = double(c[1]) * m_Feet2OSG;
		m_LargestX = double(c[2]) * m_Feet2OSG;
		m_LargestY = double(c[3]) * m_Feet2OSG;
	}
	void SetFilteredTileIndexes(int idx[]) 
	{
		for (int i = 0; i < 4; ++i)
		{
			m_FilteredTileIndexes[i] = idx[i];
		}
	}

	// reset methods
	void ResetMapType(int mt);
	void ResetConflictScale(double cs) ;
	void ResetConflictProp(double scale, 
		const std::vector<int>& shapeTypes,
		int colorCategory,
		const std::vector<std::vector<double> >& colorVecs,
		char iResetFlag);
	void ResetGridSize(double gs);
	void ResetNLevels(int n);
	void ResetIntervalColors(const std::vector<std::vector<double> >& ICVs);
	void ResetMapCoords(double c[]) ;
	void ResetColors(int cc, const std::vector<std::vector<double> >& CCVs);
private:
	HWND m_hWnd; /*!< the handle to the window for creating OSG viewer */
	HWND m_hMapDlg; /*!< the handle to Dlg_Map window for configuring map */
	std::string m_ModelName; /*!< name of the SSAMDoc */
	double m_Feet2OSG; /*!< parameter for converting feet to OSG unit */
	double m_Epsilon; /*!< parameter for floating point number comparison */
	double m_LineWidth; /*!< parameter for floating point number comparison */
	double m_ThickNessofRoad; /*!< thickness of road models */
	double m_HeightAbove; /*!< relative height to underneath surface */
	double m_SmallestX,m_SmallestY, m_SmallestZ; /*!< smallest coordinate values of conflict points */
	double m_LargestX,m_LargestY, m_LargestZ; /*!< largest coordinate values of conflict points */
	double m_ExtremeZ; /*!< largest elevation value of conflict points */
	std::vector< osg::ref_ptr<osg::Group> > m_Roots; /*!< vectors of OSG model groups */
	osg::ref_ptr<osg::Group> m_FilteredTileRoot; /*!< OSG model group of filtered tiles */
	osg::ref_ptr<osgViewer::Viewer> m_Viewer; /*!< OSG viewer */
	osg::ref_ptr<osg::MatrixTransform> m_RootNode; /*!< OSG node for rendering all models */
	osg::ref_ptr<osg::MatrixTransform> m_CameraMTF; /*!< OSG node for determining camera position */
	osg::ref_ptr<osgGA::TrackballManipulator> m_Trackball; /*!< OSG camera manipulator */ 
	osg::ref_ptr<osgGA::KeySwitchMatrixManipulator> m_KeyswitchManipulator; /*!< OSG camera manipulator switch */ 
	SP_PickParam m_pPickParam; /*!< parameters picked from OSG viewer */
	osg::ref_ptr<osgText::Font> m_Font; /*!< font for writing text */
	osg::Vec4 m_TextColor; /*!< color for writing text */
	osg::ref_ptr<osg::Vec4Array> m_RedColorVec; /*!< red color */
	osg::ref_ptr<osg::Vec4Array> m_BlackColorVec; /*!< black color */
	osg::ref_ptr<osg::Vec4Array> m_RoadColorVec; /*!< color for drawing road surface*/

	std::vector<SP_Conflict> m_ConflictList; /*!< list of conflict points to render */
	std::string m_MapFile; /*!< name of map background image file */
	/*!< coordinate values of map image: 
	 * elements are in order of minX, maxY, maxX, minY
	 */
	double m_MapCoords[4]; 
	double m_ConflictScale; /*!< parameter of conflict point models for scaling up or down */
	double m_ConflictSize; /*!< size of conflict point models */
	/*!< shape types of conflict point models 
	* index is conflict type - 1
	*/
	std::vector<int> m_OrigShapeTypes, m_ShapeTypes; 
	/*!< color category of conflict points: 
	* 0 is conflict type, 1 is conflict TTC level
	*/
	int m_ColorCategory; 
	/*!< color of conflict point models 
	* if m_ColorCategory = 0, index is TTC level index
	* if m_ColorCategory = 1, index is conflict type - 1
	*/
	std::vector<std::vector<double> > m_ConflictColorVecs;
	std::string m_ImgFileName; /*!< file name to save OSG viewer */
	bool m_IsNewCase; /*!< flag to indicate whether SSAMDoc data is new */
	int m_FilteredTileIndexes[4]; //mini, maxi, minj, maxj

	int m_MapType; /*!< map type to render */ 
	// Grid cells and tiles are generated from left to right, bottom to top
	double m_GridSize, m_OrigGridSize; /*!< size of grid cell */
	int m_NXGrids; /*!< number of grid cells along X-axis */ 
	int m_NYGrids; /*!< number of grid cells along Y-axis */ 
	std::vector<std::vector<GridInfo> > m_Grids; /*!< vector of grid cells for generating barchart map */ 
	int m_NXTiles; /*!< number of tiles along X-axis */ 
	int m_NYTiles; /*!< number of tiles along Y-axis */
	std::vector<std::vector<TileInfo> > m_Tiles; /*!< vector of tiles for filtering in map */
	int m_OrigNLevels, m_NLevels; /*!<  number of levels, one level for one contour line */
	std::vector<LevelInfo> m_Levels; /*!< vector of contour levels: stored from highest to lowest */
	/*!<  colors of contour intervals, size should be number of levels plus 1 */
	std::vector<std::vector<double> > m_IntervalColorVecs;  
	/*!<  OSG color vectors of contour intervals*/
	std::vector<osg::ref_ptr<osg::Vec4Array> > m_OSGIntervalColorVecs;
	double m_MinIntervalVal; /*!<  data value of minimum contour interval*/
	double m_MaxIntervalVal; /*!<  data value of maximum contour interval*/
	/*!<  data points to generate contour map: for each point:
	* (x, y): x and y coordinate of center point of grid; 
	* z: totalConflicts
	*/
	std::vector<point> m_DataPoints;
	point m_BBL, m_BBR, m_BTL,m_BTR; /*!< network boundary points */
	point m_GBL, m_GBR, m_GTL, m_GTR; /*!< generated grid boundary points */
	double m_Delta; /*!< height differences between contour levels */
	
	/*!<  flag to indicate which parameter is reset:
	* 0x0000: no reset; 
	* 0x0001(bit 1): scale; 
	* 0x0002(bit 2): shape; 
	* 0x0004(bit 3): color
	* 0x0008(bit 4): map coords; 
	* 0x0010(bit 5): grid size; 
	* 0x0020(bit 6): m_NLevels; 
	* 0x0040(bit 7): interval colors
	* 0x0080(bit 8): init scenegraph
	* 0x0100(bit 9): edit map
	*/
	unsigned int m_ResetFlag; 
	
	/** Initialize OSG camera manipulator.
	*/
	void InitManipulators(void);

	/** Initialize OSG viewer.
	*/
	void InitOSGViewer(void);

	/** Render OSG scene graph.
	* @param filename name of the SSAMDoc.
	* @param conflictList list of conflict points.
	* @param IsNewCase flag to indicate whether SSAMDoc data is new.
	*/
	void RenderSceneGraph(std::string filename, const std::vector<SP_Conflict>& conflictList, bool IsNewCase);

	// draw methods
	void CreateColorVectors();
	bool GenerateGrids();
	void GenerateContourLines();
	void GenerateLevels();
	void DrawConflicts(osg::ref_ptr<osg::Group> ConflictRoot);
	void DrawMap(osg::ref_ptr<osg::Group> MapRoot);
	void DrawTiles(osg::ref_ptr<osg::Group> TileRoot, 
		int fidx[], osg::ref_ptr<osg::Group> FilteredTileRoot);
	void DrawGrids(osg::ref_ptr<osg::Group> GridRoot,
		osg::ref_ptr<osg::Group> BarChartRoot);
	void DrawBarCharts(int gi, int gj, // index of grid
		int vals[], int nvals, 
		const point& center,
		float size, 
		osg::ref_ptr<osg::Group> BarChartRoot,
		std::vector<osg::ref_ptr<osg::Vec4Array> > barColors);
	void DrawContourMap(osg::ref_ptr<osg::Group> ContourRoot,
		osg::ref_ptr<osg::Group> HeatRoot);
	void DrawHeatMap(osg::ref_ptr<osg::Group> HeatRoot);
	
	// for resetting map
	void RegenerateLevels();
	void RemoveAllChildren(osg::ref_ptr<osg::Group> root);

	// redraw methods
	void RedrawScale();
	void RedrawShape();
	void RedrawColor();
	void RedrawGrids();
	void RedrawContour();
	void RedrawHeat();
	void RedrawMap();
	void RedrawBarChart(osg::ref_ptr<osg::Group> BarChartRoot);
	
	/** Process parameters picked from OSG viewer to display conflict point properties or filter data.
	*/
	void ProcessPickParam();

	/** Save current OSG viewer to an image file.
	* @param filename file name to save the OSG viewer image
	*/
	bool TakeScreenshot(const std::string& filename);

	/** Set node mask of each group according to map type.
	*/
	void SetRootMask();

	/** Find a conflict point by its index.
	* @param idx index of the conflict point to search
	* @return smart pointer to the conflict point if found, NULL otherwise
	*/
	SP_Conflict FindConflictPoint(int idx)
	{
		if (idx >= 0 && idx < m_ConflictList.size())
			return m_ConflictList[idx];
		else
			return NULL;
	}

	/** Get the coordinate of a grid cell by its indexes.
	* @param i index along X-axis of grid cell
	* @param j index along Y-axis of grid cell
	* @param BL bottom left corner of grid cell
	* @param TR top right corner of grid cell
	* @return a flag to indicate whether the grid cell is found
	*/
	bool GetGridCoords(int i, int j, double BL[], double TR[])
	{
		if (i >= 0 && i < m_NXGrids 
			&& j >= 0 && j < m_NYGrids)
		{
			GridInfo* pGrid = &(m_Grids[i][j]);
			BL[0] = pGrid->m_BL.x / m_Feet2OSG;
			BL[1] = pGrid->m_BL.y / m_Feet2OSG;
			BL[2] = pGrid->m_BL.z / m_Feet2OSG;

			TR[0] = pGrid->m_TR.x / m_Feet2OSG;
			TR[1] = pGrid->m_TR.y / m_Feet2OSG;
			TR[2] = pGrid->m_TR.z / m_Feet2OSG;

			return true;
		} else
		{
			return false;
		}
	}

	/** Get the boundary coordinate of filtered tiles by two diagonal selected tiles.
	* @param n number of filtered tiles
	* @param i indexes along X-axis of selected tiles
	* @param j indexes along Y-axis of selected tiles
	* @param minBL output minimum bottom left corner of filtered tiles
	* @param maxTR output maximum top right corner of filtered tiles
	* @param fidx output indexes of filtered tiles
	*/
	void GetFilteredTiles(int n, int i[], int j[], double minBL[], double maxTR[],
		int fidx[]);

	/** Get the Characteristics Point on the edge for the specified contour level.
	* @param i1 index in m_DataPoints vector of first vetex of the triangle edge
	* @param i2 index in m_DataPoints vector of second vetex of the triangle edge
	* @param level contour level
	* @lineSeg output points connecting contour level line segments
	*/
	void GetPointOnEdge(int i1, int i2, LevelInfo& level, 
		std::list<int>& lineSeg);

	/** Check whether there is Characteristics Point on the edge for the specified contour level.
	* points are indexes in dataPoints vector
	* @param lv contour level value
	* @param i1 index in m_DataPoints vector of first vetex of the triangle edge
	* @param i2 index in m_DataPoints vector of second vetex of the triangle edge
	* @param p output the Characteristics Point
	* @return a flag to indicate whether the Characteristics Point exists
	*/
	bool HasChPoint(double lv, int i1, int i2, point& p);
	
	/** Smooth contour level line using Bezier curve.
	* @param level the contour level to smooth
	*/
	void SmoothLines(LevelInfo& level);

	/** Generate control points for calculating Bezier curve.
	* @param p0 starting point of first line segment
	* @param p1 ending point of first line segment, also starting point of second line segment
	* @param p2 ending point of second line segment
	* @param t tense parameter
	* @param controlPoints01 output control points for first line segment
	* @param controlPoints12 output control points for second line segment
	*/
	void GenControlPoints(point p0, point p1, point p2, double t,
		std::vector<point>& controlPoints01, std::vector<point>& controlPoints12);

	/** Calculate Bezier curve segments.
	* @param nSegments number of segments to calculate
	* @param controlPoints control points of the curve
	* @param ret output points to connect the curve segments
	*/
	void CalcCurveSegs(int nSegments, std::vector<point>& controlPoints, 
		std::vector<point>& ret);

	/** Find point on network boundary from the point on grid boundary.
	* @param p0 point on grid boundary
	* @param i1 index in m_DataPoints vector of first vetex of the edge where p0 from
	* @param i2 index in m_DataPoints vector of second vetex of the edge where p0 from
	* @param side output the boundary position with values: 0: left; 1: right; 2: bottom ; 3: top
	*/
	bool FindPointOnBoundary(const point& p0, int i1, int i2,
		point& p, int& side);

	
	/** Calculate the offset for a point perpendicular to the line between
	* point 1 and and point 2 with a specified distance from the line,
	* returned point is on the right side of direction P2->P1
	* @param x1 x value of starting point p1
	* @param y1 y value of starting point p1
	* @param x2 x value of ending point p2
	* @param y2 y value of ending point p2
	* @param dist Distance of the perpendicular offset
	* @return Returns offset vector as a point object
	*/
	point CalcPerpOffset(double x1,
						 double y1,
						 double x2,
						 double y2,
						 double dist);

	/** Draw line with custome width
	* @param startPoint Starting point of line
	* @param endPoint ending point of line
	* @param width width of line
	* @param heightAbove height of line relative to underneath surface
	* @param root pointer to OSG group for drawn line model
	* @param colors pointer to color vector to draw lie
	*/
	void DrawACustomWidthLine(point startPoint,
							  point endPoint,
							  double width,
							  double heightAbove,
							  osg::Group* root,
							  osg::Vec4Array* colors);

	/** Write text with custom size at designated position
	* @param text text to write
	* @param position position to write the text
	* @param characterSize size of text characters
	* @param color reference to color vector to write text
	* @param textRoot pointer to OSG group for text model
	*/
	void WriteText(const std::string& text, 
		point position, 
		float characterSize,
		const osg::Vec4& color,
		osg::ref_ptr<osg::Group> textRoot);

	/** Draw a quad in 3D space with a specified color
	* @param BL Bottom left corner of quad
	* @param BR Bottom right corner of quad
	* @param TL Top left corner of quad
	* @param TR Top right corner of quad
	* @param root pointer to OSG group for drawn quad model
	* @param colors pointer to color vector to draw quad
	* @param name name of drawn quad model
	*/
	void DrawQuad(point BL, point BR, point TL, point TR,
				   osg::Group* root, osg::Vec4Array* colors, 
				   const std::string& name = "");

	/** Draw a triangle in 3D space with a specified color
	* @param p1 first vertex of triangle
	* @param p2 second vertex of triangle
	* @param p3 third vertex of triangle
	* @param root pointer to OSG group for drawn triangle model
	* @param colors pointer to color vector to draw triangle
	* @param name name of drawn triangle model
	*/
	void DrawTri(point p1, point p2, point p3, 
		osg::Group* root, osg::Vec4Array* colors,
		const std::string& name = "");

	/** Calculate the centroid for a polygon
	* @param sps array of shape points of the polygon
	* @return the centroid point
	*/
	point CalcPolygonCentroid(const std::vector<point>& sps);

	/** Calculate distance between two points
	* @param p1 coordinate of first point
	* @param p2 coordinate of second point
	* @return distance in OSG unit
	*/
	double CalcP2PDist(const point& p1, const point& p2)
	{
		return CalcP2PDist(p1.x, p1.y, p2.x, p2.y);
	}

	/** Calculate distance between two points
	* @param x1 x coordinate of first point
	* @param y1 y coordinate of first point
	* @param x2 x coordinate of second point
	* @param y2 y coordinate of second point
	* @return distance in OSG unit
	*/
	double CalcP2PDist(double x1, double y1, double x2, double y2)
	{
		double dx = x1 - x2;
		double dy = y1 - y2;
		return sqrt(dx * dx + dy * dy);
	}

	/** Find a point on line with distance scalar to the starting point
	* @param p1 coordinate of starting point of line
	* @param p2 coordinate of ending point of line
	* @param scalar coordinate of ending point of line
	* @return the point on line
	*/
	point FindPointByScalar(point p1, point p2, double scalar);
};

/** CRenderingThread implements an OpenThreads::Thread 
* to manage SSAMOSG instance.
*/
class CRenderingThread : public OpenThreads::Thread
{
public:
	/** Constructor creates a CRenderingThread object 
	* @param ptr pointer to a SSAMOSG instance.
	*/
    CRenderingThread( SSAMOSG* ptr );
    virtual ~CRenderingThread();

	/** Implement virtual run method. 
	*/
    virtual void run();

	// set() methods pass parameters to SSAMOSG instance
	void SetImageFileName(const std::string& f) {_ptr->SetImageFileName( f);}
	void SetMapType(int mt) { _ptr->SetMapType(mt);	}
	void SetMapFile(const std::string& f) {_ptr->SetMapFile(f);}
	void SetMapCoords(double c[]) { _ptr->SetMapCoords(c);}
	void SetConflictScale(double cs) {_ptr->SetConflictScale(cs);}
	void SetShapeTypes(const std::vector<int>& STs) {_ptr->SetShapeTypes( STs); }
	void SetColorCategory(int cc) {_ptr->SetColorCategory(cc);}
	void SetColorVecs(const std::vector<std::vector<double> >& CCVs) 
	{_ptr->SetColorVecs( CCVs);	}
	void SetGridSize(double gs) { _ptr->SetGridSize(gs);}
	void SetNLevels(int nl) { _ptr->SetNLevels(nl);}
	void SetIntervalColorVecs(const std::vector<std::vector<double> >& ICVs)
	{_ptr->SetIntervalColorVecs(ICVs);	}
	void SetMapBoundaries(int c[]) {_ptr->SetMapBoundaries(c);	}
	void SetFilteredTileIndexes(int idx[]) {_ptr->SetFilteredTileIndexes(idx);}

	// reset methods update parameters to SSAMOSG instance
	void ResetMapType(int mt) {_ptr->ResetMapType(mt);}
	void ResetConflictScale(double cs) {_ptr->ResetConflictScale( cs);}
	void ResetConflictProp(double cs, 
		const std::vector<int>& STs,
		int cc,
		const std::vector<std::vector<double> >& CCVs,
		char ResetFlag)
	{_ptr->ResetConflictProp(cs, STs, cc, CCVs, ResetFlag);	}
	void ResetMapCoords(double c[])  {_ptr->ResetMapCoords( c);}
	void ResetGridSize(double gs)  {_ptr->ResetGridSize( gs);}
	void ResetNLevels(int n)  {_ptr->ResetNLevels( n);}
	void ResetIntervalColorVecs(const std::vector<std::vector<double> >& ICVs)  
	{_ptr->ResetIntervalColors( ICVs);}
	void ResetViewer() {_ptr->ResetViewer();}
	void ResetColors(int cc, const std::vector<std::vector<double> >& CCVs)
	{_ptr->ResetColors(cc, CCVs);	}
	
	// pass commands to SSAMOSG instance
	void SetSceneGraph(std::string filename, const std::list<SP_Conflict>& conflictList, bool IsNewCase)
	{_ptr->SetSceneGraph(filename, conflictList, IsNewCase); }
	void EditMap(const std::string& f) {_ptr->EditMap(f);}
	void SwitchMap(bool IsShown) {_ptr->SwitchMap(IsShown); }
protected:
    SSAMOSG* _ptr; /*!< pointer to a SSAMOSG instance. */
    bool _done; /*!< flag to indicate whether SSAMOSG instance is completed. */
};

/** SaveImageDrawCallback implements an osg::Camera::DrawCallback 
* to save current OSG viewer to an image file.
*/
class SaveImageDrawCallback : public osg::Camera::DrawCallback
{
public:
    SaveImageDrawCallback()
    {
		m_IsSaveImage = false;
    }

    void SetFileName(const std::string& filename) { m_Filename = filename; }
    void SetIsSaveImage(bool flag) { m_IsSaveImage = flag; }

    virtual void operator () (const osg::Camera& camera) const
    {
        if (!m_IsSaveImage) return;

        int x,y,width,height;
        x = camera.getViewport()->x();
        y = camera.getViewport()->y();
        width = camera.getViewport()->width();
        height = camera.getViewport()->height();

        osg::ref_ptr<osg::Image> image = new osg::Image;
        image->readPixels(x,y,width,height,GL_RGB,GL_UNSIGNED_BYTE);
        osgDB::writeImageFile(*image,m_Filename);
        m_IsSaveImage = false;
    }
protected:
    std::string m_Filename; /*!< file name to save the image */
    mutable bool m_IsSaveImage; /*!< flag to indicate whether to save OSG viewer as image */
};
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
#include <string>
#include <vector>
#include "SSAMDoc.h"
#include "Dlg_Tab.h"

/** TTable stores critical values of t-distribution
  * for pairs of significant level and degree of freedom.
*/
class TTable {
public:
	/** Constructor reads in TTable file 
	*/
	TTable();
	~TTable(){}

	/** Constructor reads t critical value with significant level and degree of freedom.
	* @param sigLevel significant level
	* @param degreeFreedom degree of freedom
	*/
	TTable(const std::string& sigLevel, int degreeFreedom);

	float GetTCritical() const { return m_TCritical;}
private:
	std::string m_TTableFile; /*!< Name of TTable file */
	/*!< A 2-dimensional array stores t critical values, rows in degree of freedom, columns in significant level */
	std::vector<std::vector<float> > m_Datas; 
	float m_TCritical; /*!< t critical value with given significant level and degree of freedom. */
	
	/** Parse the TTable file and store t critical values.
	* @param tTFile Name of TTable file
	*/
	void ReadTTableFile(const std::string& tTFile); 

	/** Read t critical value with significant level and degree of freedom.
	* @param colidx the column index for the input significant level
	* @param degreeFreedom degree of freedom
	* @return t critical with given significant level and degree of freedom.
	*/
	float ReadTCritical(int colidx,int degreeFreedom);

	/** Check whether input string is a valid number.
	* @param str input string
	* @return true if input string is a valid number.
	*/
	bool IsValidNumber(const std::string& str);
};
/** Smart pointer type to TTable class.
  */
typedef std::shared_ptr<TTable> SP_TTable;

/** FTable stores critical values of f-distribution
  * for combinations of significant level and degrees of freedom.
*/
class FTable 
{
public:
	FTable(){}
	~FTable(){}
	/** Constructor reads f critical value with significant level and degrees of freedom.
	* @param sigLevel significant level
	* @param degreeOne numerator degree of freedom
	* @param degreeTwo denominator degree of freedom
	*/
	FTable(const std::string& sigLevel, int degreeOne,int degreeTwo);

	float GetFCritical() const { return m_FCritical;}
private:
	std::string m_FTFile; /*!< Name of FTable file for given significant level*/
	/*!< A 2-dimensional array stores t critical values, 
	 * rows in denominator degree of freedom, columns in numerator degree of freedom 
	 */
	std::vector<std::vector<float> > m_Datas;
	float m_FCritical; /*!< f critical value with given significant level and degrees of freedom. */
	
	/** Parse the FTable file and store f critical values.
	* @param fTFile Name of FTable file for given significant level
	*/
	void ReadFTableFile(const std::string& fTFile); 

	/** Read f critical value with degrees of freedom.
	* @param degree1 numerator degree of freedom
	* @param degree2 denominator degree of freedom
	* @return f critical with degrees of freedom.
	*/
	float ReadFCritical(int degree1,int degree2);

	/** Round a float number to integer.
	* @param x float number
	* @return rounded integer number.
	*/
	inline int GetInt(float x)
	{
		return int(x+0.5);
	}
};
/** Smart pointer type to FTable class.
  */
typedef std::shared_ptr<FTable> SP_FTable;

/** TTestRunner calculates statistical values and runs t-test.
*/
class TTestRunner
{
public:
	/** Constructor creates a TTestRunner object.
	* @param pSSAMDoc smart pointer to the first SSAM case
	* @param pSSAMDoc2 smart pointer to the second SSAM case
	* @param isFiltered flag indicates whether to use filtered data
	*/
	TTestRunner(SP_SSAMDoc pSSAMDoc, SP_SSAMDoc pSSAMDoc2, bool isFiltered = false);
	~TTestRunner(){}

	/** Run a t-test.
	* @param tSigLevel significant level of t-test
	* @param fSigLevel significant level of f-test
	*/
	void RunTTest(const std::string& tSigLevel, const std::string& fSigLevel);

	std::vector<float> m_TTestResults; /*!< An array stores t-test results.*/
	std::vector<float> m_MeanDiff; /*!< An array stores mean differences.*/
	std::vector<float> m_TCritical; /*!< An array stores t critical values.*/
	std::vector<std::string> m_SigResults; /*!< An array stores significant results.*/
	int n1; /*!< Number of samples in the first SSAM case.*/
	std::vector<float> mean1; /*!< An array stores means of values in first SSAM case.*/
	std::vector<float> var1; /*!< An array stores variances of values in first SSAM case.*/
	int n2; /*!< Number of samples in the second SSAM case.*/
	std::vector<float> mean2; /*!< An array stores means of values in second SSAM case.*/
	std::vector<float> var2; /*!< An array stores variances of values in second SSAM case.*/
protected:
	SP_SSAMDoc m_pSSAMDoc; /*!< smart pointer to the first SSAM case */
	SP_SSAMDoc m_pSSAMDoc2; /*!< smart pointer to the second SSAM case */
	bool m_IsFiltered; /*!< flag indicates whether to use filtered data */
	
	/** Calculate statistical values.
	*/
	virtual void CalculateValues() = 0;
};

/** Smart pointer type to TTestRunner class.
*/
typedef std::shared_ptr<TTestRunner> SP_TTestRunner;

/** MeasureTTestRunner calculates statistical values and runs t-test for safety measures.
*/
class MeasureTTestRunner : public TTestRunner
{
public:
	MeasureTTestRunner(SP_SSAMDoc pSSAMDoc, SP_SSAMDoc pSSAMDoc2, bool isFiltered = false)
		: TTestRunner(pSSAMDoc, pSSAMDoc2, isFiltered)
	{
	}
	~MeasureTTestRunner(){}
protected:
	/** Implement the virtual function to calculate statistical values.
	*/
	virtual void CalculateValues();
};
/** Smart pointer type to MeasureTTestRunner class.
*/
typedef std::shared_ptr<MeasureTTestRunner> SP_MeasureTTestRunner;


/** CFNumTTestRunner calculates statistical values and runs t-test for conflict numbers.
*/
class CFNumTTestRunner : public TTestRunner
{
public:
	CFNumTTestRunner(SP_SSAMDoc pSSAMDoc, SP_SSAMDoc pSSAMDoc2, bool isFiltered = false)
		: TTestRunner(pSSAMDoc, pSSAMDoc2, isFiltered)
	{
	}
	~CFNumTTestRunner(){}
protected:
	/** Implement the virtual function to calculate statistical values.
	*/
	virtual void CalculateValues();
private:
	/** Calculate mean values.
	* @param data 2-dimensional array of data; first dimension is sample, second dimenstion is measure
	* @param meanVals output mean values 
	*/
	void CalcMean(const std::vector<std::vector<int> >& data, std::vector<float>& meanVals);

	/** Calculate variance values.
	* @param data 2-dimensional array of data; first dimension is sample, second dimenstion is measure
	* @param meanVals mean values of measures
	* @param varVals output variance values 
	*/
	void CalcVar(const std::vector<std::vector<int> >& data, 
		const std::vector<float>& meanVals,
		std::vector<float>& varVals);
};
/** Smart pointer type to CFNumTTestRunner class.
*/
typedef std::shared_ptr<CFNumTTestRunner> SP_CFNumTTestRunner;

// CDlg_TTest dialog

class CDlg_TTest : public CDlg_Tab
{
	DECLARE_DYNAMIC(CDlg_TTest)

public:
	CDlg_TTest(CWnd* pParent = NULL);   // standard constructor
	CDlg_TTest(SP_SSAMDoc pSSAMDoc, CWnd* pParent = NULL); 
	virtual ~CDlg_TTest();

// Dialog Data
	enum { IDD = IDD_DIALOG_TTEST };

	virtual BOOL OnInitDialog();
	virtual bool SaveData();
	virtual void SetData();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	SP_SSAMDoc m_pSSAMDoc, m_pSSAMDoc2;
	SP_MeasureTTestRunner m_pMeasTTest;
	SP_CFNumTTestRunner m_pCFNumTTest;
	std::vector<int> m_SumMeasures;
	std::vector<std::string> m_ConflictTypeLabels;
	std::vector<std::string> m_TTestSigLevels, m_FTestSigLevels;
	std::vector<std::string> m_Measheaders, m_CFNumheaders;
	int m_MeasureRows;
	int m_CFNumHeaderRow;

	CStatic m_Static_File1, m_Static_File2, m_Static_Msg;
	CComboBox m_Combo_SigLevel, m_Combo_FSigLevel;
	CListBox m_ListBox_DataSet;
	CListCtrl m_List_Measures, m_List_Conflicts;

	void PickSumMeasures()
	{ 
		m_SumMeasures.clear();
		for(int i=0; i<Conflict::NUM_MEASURES; i++)
		{
			if(Conflict::SUM_MEASURES[i])
			{
				m_SumMeasures.push_back(i);
			}
		}
	}

	void DisplayResults();
	std::string GetValueAt(int row, int col);
	std::string GetFloatString(float x)
	{
		double y = (x >= 0) ? float(int(x * 100.0 + 0.5)) / 100.0 : float(int(x * 100.0 - 0.5)) / 100.0;
		std::string str(std::to_string(y));
		std::size_t pos = str.find('.');
		if (pos != std::string::npos)
		{
			str = str.substr(0, pos + 3);
		}
		return str;
	}

public:
	afx_msg void OnBnClickedButtonAddfile();
	afx_msg void OnBnClickedButtonDeletefile();
	afx_msg void OnBnClickedButtonAnalyze();
	afx_msg void OnBnClickedButtonExport();
};

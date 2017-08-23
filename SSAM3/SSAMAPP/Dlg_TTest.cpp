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
// Dlg_TTest.cpp : implementation file
//

#include "stdafx.h"
#include "SSAMAPP.h"
#include "Dlg_TTest.h"
#include "afxdialogex.h"
#include <iterator>
#include <sstream>

// CDlg_TTest dialog

IMPLEMENT_DYNAMIC(CDlg_TTest, CDialog)

CDlg_TTest::CDlg_TTest(CWnd* pParent /*=NULL*/)
	: CDlg_Tab(pParent)
{

}

CDlg_TTest::CDlg_TTest(SP_SSAMDoc pSSAMDoc, 
						 CWnd* pParent /*=NULL*/)
	: CDlg_Tab(pParent)
	, m_pSSAMDoc (pSSAMDoc)
	, m_pSSAMDoc2(NULL)
{
	
}

CDlg_TTest::~CDlg_TTest()
{
}

void CDlg_TTest::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_STATIC_FILE1, m_Static_File1);
	DDX_Control(pDX, IDC_STATIC_FILE2, m_Static_File2);
	DDX_Control(pDX, IDC_STATIC_MSG, m_Static_Msg);

	DDX_Control(pDX, IDC_COMBO_ALPHA, m_Combo_SigLevel);
	DDX_Control(pDX, IDC_COMBO_FALPHA, m_Combo_FSigLevel);
	DDX_Control(pDX, IDC_LIST_DATASET, m_ListBox_DataSet);

	DDX_Control(pDX, IDC_LIST_MEASURES, m_List_Measures);
	DDX_Control(pDX, IDC_LIST_CONFLICTS, m_List_Conflicts);
}

BOOL CDlg_TTest::OnInitDialog()
{
	CDialog::OnInitDialog();

	PickSumMeasures();
	m_MeasureRows = m_SumMeasures.size();
	m_CFNumHeaderRow = m_MeasureRows + 1;

	m_ConflictTypeLabels.push_back("Crossing");
	m_ConflictTypeLabels.push_back("Rear-end");
	m_ConflictTypeLabels.push_back("Lane changing");
	m_ConflictTypeLabels.push_back("Total");

	m_TTestSigLevels.push_back( "0.0005");
	m_TTestSigLevels.push_back( "0.001");
	m_TTestSigLevels.push_back( "0.0025");
	m_TTestSigLevels.push_back( "0.005");
	m_TTestSigLevels.push_back( "0.01");
	m_TTestSigLevels.push_back( "0.02");
	m_TTestSigLevels.push_back( "0.025");
	m_TTestSigLevels.push_back( "0.05");
	m_TTestSigLevels.push_back( "0.10");
	m_TTestSigLevels.push_back( "0.15");
	m_TTestSigLevels.push_back( "0.20");
	m_TTestSigLevels.push_back( "0.25");
	for (std::vector<std::string>::iterator it = m_TTestSigLevels.begin(); it != m_TTestSigLevels.end(); ++it)
	{
		m_Combo_SigLevel.AddString(it->c_str());
	}
	m_Combo_SigLevel.SetCurSel(7); // default as 0.05

	m_FTestSigLevels.push_back("0.01");
	m_FTestSigLevels.push_back("0.025");
	m_FTestSigLevels.push_back("0.05");
	m_FTestSigLevels.push_back("0.1");
	for (std::vector<std::string>::iterator it = m_FTestSigLevels.begin(); it != m_FTestSigLevels.end(); ++it)
	{
		m_Combo_FSigLevel.AddString(it->c_str());
	}
	m_Combo_FSigLevel.SetCurSel(0);

	m_ListBox_DataSet.AddString("All Data");
	m_ListBox_DataSet.AddString("Filtered Data");
	m_ListBox_DataSet.SetCurSel(0);
	
	// add the headers
	m_List_Measures.SetExtendedStyle(m_List_Measures.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	for (int i = 0; i < 11; i++)
	{
		m_List_Measures.InsertColumn(i, _T(""), LVCFMT_LEFT, 90, -1);
	}

	return TRUE;
}

bool CDlg_TTest::SaveData()
{
	return true;
}

void CDlg_TTest::SetData()
{
	m_Static_File1.SetWindowText(m_pSSAMDoc->GetDocName().c_str());
}


BEGIN_MESSAGE_MAP(CDlg_TTest, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_ADDFILE, &CDlg_TTest::OnBnClickedButtonAddfile)
	ON_BN_CLICKED(IDC_BUTTON_DELETEFILE, &CDlg_TTest::OnBnClickedButtonDeletefile)
	ON_BN_CLICKED(IDC_BUTTON_ANALYZE, &CDlg_TTest::OnBnClickedButtonAnalyze)
	ON_BN_CLICKED(IDC_BUTTON_EXPORT, &CDlg_TTest::OnBnClickedButtonExport)
END_MESSAGE_MAP()


// CDlg_TTest message handlers


void CDlg_TTest::OnBnClickedButtonAddfile()
{
	CFileDialog dlg(TRUE, 0, 0, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT|OFN_LONGNAMES|OFN_ENABLESIZING,
		_T("*.*|*.*|"),NULL,0,true);
	if(dlg.DoModal() == IDOK)
	{
		std::string SSAMDocFile2 = dlg.GetPathName();
		m_pSSAMDoc2 = std::make_shared<SSAMDoc>();
		m_pSSAMDoc2->Open(SSAMDocFile2);
		m_Static_File2.SetWindowText(SSAMDocFile2.c_str());
	}
}


void CDlg_TTest::OnBnClickedButtonDeletefile()
{
	m_Static_File2.SetWindowText("");
	m_pSSAMDoc2 = NULL;
}

void CDlg_TTest::OnBnClickedButtonAnalyze()
{
	UpdateData(TRUE);

	if (m_pSSAMDoc2 == NULL)
	{
		AfxMessageBox("Please Add another SSAM file for analysis! ");
		return;
	}

	std::string curTSigLevel = m_TTestSigLevels[m_Combo_SigLevel.GetCurSel()]; 
	std::string curFSigLevel = m_FTestSigLevels[m_Combo_FSigLevel.GetCurSel()]; 

	int curDataset = m_ListBox_DataSet.GetCurSel(); //0: all data, 1: filtered data
	if (curDataset == 1 && (!m_pSSAMDoc->IsFilterApplied() || !m_pSSAMDoc2->IsFilterApplied()))
	{
		AfxMessageBox( "There's no filtering data for one or two of the case files!");	
		return;
	}

	m_Static_Msg.SetWindowText("t test for all data has been applied"); 
	if (curDataset == 1)
	{
		m_Static_Msg.SetWindowText("t test for filtered data has been applied");
	} 
	
	try
	{
		m_pMeasTTest = std::make_shared<MeasureTTestRunner>(m_pSSAMDoc, m_pSSAMDoc2, curDataset == 1);
		m_pCFNumTTest = std::make_shared<CFNumTTestRunner>(m_pSSAMDoc, m_pSSAMDoc2, curDataset == 1);

		m_pMeasTTest->RunTTest(curTSigLevel, curFSigLevel);
		m_pCFNumTTest->RunTTest(curTSigLevel, curFSigLevel);
	} catch (std::runtime_error& e)
	{
		std::string errMsg(e.what());
		AfxMessageBox(errMsg.c_str());
		return;
	}
	DisplayResults();
}

void CDlg_TTest::OnBnClickedButtonExport()
{
	if (m_Measheaders.empty())
	{
		AfxMessageBox("No t-test result exists.");
		return;
	}
	std::string fileName(m_pSSAMDoc->GetCsvName());
	fileName = fileName.substr(0, fileName.length() - 4) + "_ttest.csv";
	CFileDialog fdlg (FALSE, "*.csv", fileName.c_str(),OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT |OFN_ENABLESIZING | OFN_EXPLORER,
			"CSV (Comma delimited) (*.csv)|*.csv|",NULL,0,true);	

	if(fdlg.DoModal()==IDOK)
	{
		CString path = fdlg.GetPathName  ();
		std::ofstream csvFile(path);

		// insert the column headers 
		for(int iCol = 0; iCol < m_Measheaders.size(); iCol++)
		{
			csvFile << m_Measheaders[iCol] << ",";
		}
		csvFile << std::endl;
	
		// Two extra rows: one is empty row, the other is Conflict header row
		int TotalRows = m_SumMeasures.size() + m_ConflictTypeLabels.size() + 2;
		for (int iRow = 0; iRow < TotalRows; ++iRow)
		{
			for (int iCol = 0; iCol < m_Measheaders.size(); ++iCol)
			{
				csvFile << GetValueAt(iRow, iCol) << ",";
			}
			csvFile << std::endl;
		}
		
		csvFile.close();
	}
}


void CDlg_TTest::DisplayResults()
{
	std::string Meanoffirstfile="Mean("+m_pSSAMDoc->GetDocName()+")";
	std::string Varoffirstfile="Variance ("+m_pSSAMDoc->GetDocName()+")";
	std::string SampleNumOne="Replications ("+m_pSSAMDoc->GetDocName()+")";
	std::string Meanofsecondfile="Mean("+m_pSSAMDoc2->GetDocName()+")";
	std::string Varofsecondfile="Variance ("+m_pSSAMDoc2->GetDocName()+")";
	std::string SampleNumtwo="Replications ("+m_pSSAMDoc2->GetDocName()+")";
	
	m_Measheaders.clear();
	m_Measheaders.push_back("SSAM Measures");
	m_Measheaders.push_back(Meanoffirstfile);
	m_Measheaders.push_back(Varoffirstfile);
	m_Measheaders.push_back(SampleNumOne);
	m_Measheaders.push_back(Meanofsecondfile);
	m_Measheaders.push_back(Varofsecondfile);
	m_Measheaders.push_back(SampleNumtwo);
	m_Measheaders.push_back("t value");
	m_Measheaders.push_back("t critical");
	m_Measheaders.push_back("Significant");
	m_Measheaders.push_back("Mean Difference");

	m_CFNumheaders = m_Measheaders;
	m_CFNumheaders[0]= "Conflict Types";

	m_List_Measures.DeleteAllItems();
	int nColumnCount = m_List_Measures.GetHeaderCtrl()->GetItemCount();
	// Delete all of the columns.
	for (int i=0; i < nColumnCount; i++)
	{
		m_List_Measures.DeleteColumn(0);
	}

	// insert the column headers 
	for(int iCol = 0; iCol < m_Measheaders.size(); iCol++)
	{
		m_List_Measures.InsertColumn(iCol, _T(m_Measheaders[iCol].c_str()), LVCFMT_LEFT, 90, -1);
	}
	
	// Two extra rows: one is empty row, the other is Conflict header row
	int TotalRows = m_SumMeasures.size() + m_ConflictTypeLabels.size() + 2;
	int iRow = 0;
	char tmpStr[1000];
	for (int iRow = 0; iRow < TotalRows; ++iRow)
	{
		int iCol = 0;
		int nIdx = m_List_Measures.InsertItem(iRow, _T(GetValueAt(iRow, iCol).c_str()));
		for (iCol = 1; iCol < m_Measheaders.size(); ++iCol)
		{
			m_List_Measures.SetItemText(nIdx, iCol, _T(GetValueAt(iRow, iCol).c_str()));
		}
	}
}

std::string CDlg_TTest::GetValueAt(int row, int col)
{
	// empty row
	if (row == m_MeasureRows)
		return "";
	
	// Conflict header row
	if (row == m_CFNumHeaderRow)
		return m_CFNumheaders[col];

	int dataIdx = 0;
	SP_TTestRunner pTTest = NULL;
	if(row < m_MeasureRows)
	{
		dataIdx = m_SumMeasures[row];	
		pTTest = m_pMeasTTest;
	} else 
	{
		dataIdx = row - m_CFNumHeaderRow - 1;
		pTTest = m_pCFNumTTest;
	}

	switch (col)
	{
		case 0:
			if(row < m_MeasureRows)
				return Conflict::MEASURE_LABEL[dataIdx];						
			else 
				return m_ConflictTypeLabels[dataIdx];							
		case 1:
			return GetFloatString(pTTest->mean1[dataIdx]);					
		case 2:
			return GetFloatString(pTTest->var1[dataIdx]);										
		case 3:
			return std::to_string(pTTest->n1);					
		case 4:
			return GetFloatString(pTTest->mean2[dataIdx]);											       
		case 5:
			return GetFloatString(pTTest->var2[dataIdx]);												
		case 6:
			return std::to_string(pTTest->n2);												
		case 7:
			return GetFloatString(pTTest->m_TTestResults[dataIdx]);						
		case 8:
			return GetFloatString(pTTest->m_TCritical[dataIdx]);						
		case 9:
			return pTTest->m_SigResults[dataIdx];													
		case 10:
			return GetFloatString(pTTest->m_MeanDiff[dataIdx]);					
		default: return "N/A";					
	}
}


TTestRunner::TTestRunner(SP_SSAMDoc pSSAMDoc, SP_SSAMDoc pSSAMDoc2, bool isFiltered)
	: m_pSSAMDoc (pSSAMDoc)
	, m_pSSAMDoc2 (pSSAMDoc2)
	, m_IsFiltered (isFiltered)
	, n1 (0)
	, n2 (0)
{
}

void TTestRunner::RunTTest(const std::string& tSigLevel, const std::string& fSigLevel)
{
	CalculateValues();
	//prepare result sets
	//output items in ttable
	m_TTestResults.resize(mean1.size(), 0.0);
	m_MeanDiff.resize(mean1.size(), 0.0);
	m_TCritical.resize(var1.size(), 0.0);
	m_SigResults.resize(var1.size());

	// Run ttest
	std::vector<bool> direct_ttest(var1.size(), false);//index of use t test or Behrens_Fisher test	
	std::vector<float> v_ratio(var2.size(), 0.0);//for F test only
	SP_FTable UBFTable, LBFTable;
	float UBFCritical = 0; //right side critical value (upper bound)
	float LBFCritical = 0; //left side critical value (lower bound)
	std::vector<int> degree_freedom(var1.size(), 0);
	SP_TTable ttable;

	//check sample number to determine wheather or not to use F test
	//if both variances are zero, we consider F test is non-sig,;if either of them is zero, F test is sig 
	if (n1==n2)
	{
		for (int i=0; i<var1.size();i++)
		{
			if((var1[i]==0 & var2[i]!=0)|(var2[i]==0 & var1[i]!=0))
				direct_ttest[i]=false;
			else
				direct_ttest[i]=true;
		}			
	}
	else 
	{
		UBFTable=std::make_shared<FTable>(fSigLevel,n1-1,n2-1);
		LBFTable=std::make_shared<FTable>(fSigLevel,n2-1,n1-1);
		UBFCritical=UBFTable->GetFCritical();
		LBFCritical=1.0/(LBFTable->GetFCritical());
		for (int i=0; i<var1.size();i++)
		{
			if((var1[i]==0 & var2[i]!=0)|(var2[i]==0 & var1[i]!=0))
				direct_ttest[i]=false;
			else if(var1[i]==0 & var2[i]==0)
				direct_ttest[i]=true;
			else
			{
				v_ratio[i]=var1[i]/var2[i];
				if (v_ratio[i]<=UBFCritical & v_ratio[i]>=LBFCritical)
				{
					direct_ttest[i]=true;
				}
				else 
				{
					direct_ttest[i]=false;
				}
			}		
		}
	}
		
	//start statistic analysis	
	//t test for safety measures
	//if both mean and variance are zero (0,0,0,0), no sig
	//if both variance are zero and means are diff,sig
	for (int i=0; i<var1.size();i++)
	{
		m_MeanDiff[i]=(float) (mean1[i]-mean2[i]);
		if(direct_ttest[i]==true)
		{
			degree_freedom[i]=n1+n2-2;	
			float pooled_var=((float)(n1-1)*var1[i]+(float)(n2-1)*var2[i])/(float)(n1+n2-2);
			m_TTestResults[i]=(float) ((mean1[i]-mean2[i])/(sqrt((double)(pooled_var*(float)1/(float)n1+(float)1/(float)n2))));
		}
		else
		{
			degree_freedom[i]=(int)(pow((var1[i]/((float)(n1)))+(var2[i]/((float)(n2))),2.0)
					/((pow(var1[i]/(n1),2.0)/(n1+1))
					+(pow(var2[i]/(n2),2.0)/(n2+1))))-2;	
			m_TTestResults[i]=(float) ((mean1[i]-mean2[i])/sqrt((double)((var2[i]/(float)n2)+(var1[i]/(float)n1))));
				
		}
		ttable=std::make_shared<TTable>(tSigLevel,degree_freedom[i]);
		m_TCritical[i]=ttable->GetTCritical();
		if (abs(m_TTestResults[i])<= abs(m_TCritical[i]))
		{
			m_SigResults[i]="NO";
		}
		else
		{
			m_SigResults[i]="YES";
		}
		if(var1[i]==0&&var2[i]==0)//when both variance are zero
		{
			if(m_MeanDiff[i]==0)//when both mean are zero OR same mean
			{
				m_TTestResults[i]=0;
				m_SigResults[i]="NO";
			}
			else
				m_SigResults[i]="YES";									
		}
			
			
	}
}

void MeasureTTestRunner::CalculateValues()
{
	SP_Summary pSummary1 = m_pSSAMDoc->GetSummary();
	SP_Summary pSummary2 = m_pSSAMDoc2->GetSummary();
	
	if (m_IsFiltered)
	{
		pSummary1 = m_pSSAMDoc->GetFilteredSummary();
		pSummary2 = m_pSSAMDoc2->GetFilteredSummary();
	} 

	std::vector<int> numConflicts1 = pSummary1->GetConflictTypeCounts(); // only need a total num conflicts, can be passed as two parameters
	std::vector<int> numConflicts2 = pSummary2->GetConflictTypeCounts(); 
	int numConflictTypes = pSummary1->GetConflictTypeCounts().size();

	n1 = numConflicts1[numConflictTypes-1];//sample size 1 for F and T test of safety measures
	n2 = numConflicts2[numConflictTypes-1];//sample size 2 for F and T test of safety measures
	
	//check sample number for ttest; change to throw exception
	if(n1<2)
	{
		std::string tmpstr(m_pSSAMDoc->GetDocName() + " has only one sample data for safety measures, not enough for ttest!");
		throw(SSAMException(tmpstr));
	}
	else if(n2<2)
	{
		std::string tmpstr(m_pSSAMDoc2->GetDocName() + " has only one sample data for safety measures, not enough for ttest!");
		throw(SSAMException(tmpstr));
	}
	
	// prepare values of safety measures
	mean1 = pSummary1->GetMeanVals();
	var1 = pSummary1->GetVarVals();
	mean2 = pSummary2->GetMeanVals();
	var2 =pSummary2->GetVarVals();
}

void CFNumTTestRunner::CalculateValues()
{
	SP_Summary pSummary1 = m_pSSAMDoc->GetSummary();
	SP_Summary pSummary2 = m_pSSAMDoc2->GetSummary();
	std::list<SP_Summary>* pSummaries1 = &(m_pSSAMDoc->GetSummaries());
	std::list<SP_Summary>* pSummaries2 = &(m_pSSAMDoc2->GetSummaries());
	
	if (m_IsFiltered == 1)
	{
		pSummary1 = m_pSSAMDoc->GetFilteredSummary();
		pSummary2 = m_pSSAMDoc2->GetFilteredSummary();
		pSummaries1 = &(m_pSSAMDoc->GetFilteredSummaries());
		pSummaries2 = &(m_pSSAMDoc2->GetFilteredSummaries());
	} 
	 
	int numConflictTypes = pSummary1->GetConflictTypeCounts().size();

	// skip summary for "all" 
	std::vector<SP_Summary> fileSummaries1(std::next(pSummaries1->begin()), pSummaries1->end());
	int numTrjFiles1 = fileSummaries1.size(); 
	std::vector<std::vector<int> > fileConflicts1(numTrjFiles1, std::vector<int>(numConflictTypes-1, 0));
	
	// skip summary for "all"
	std::vector<SP_Summary> fileSummaries2(std::next(pSummaries2->begin()), pSummaries2->end());
	int numTrjFiles2 = fileSummaries2.size();
	std::vector<std::vector<int> > fileConflicts2(numTrjFiles2, std::vector<int>(numConflictTypes-1, 0));
	
	n1 = numTrjFiles1;//sample size 1 for F and T test of conflicts numbers
	n2 = numTrjFiles2;	//sample size 2 for F and T test of conflicts numbers
		
	//check sample number for ttest
	if(n1<2)
	{
		std::string tmpstr(m_pSSAMDoc->GetDocName() + " has only one iteration, not enough for ttest!");
		throw(SSAMException(tmpstr));
	}
	else if(n2<2)
	{
		std::string tmpstr(m_pSSAMDoc2->GetDocName() + " has only one iteration, not enough for ttest!");
		throw(SSAMException(tmpstr));
	}
	
	mean1.resize(numConflictTypes-1, 0.0);
	mean2.resize(numConflictTypes-1, 0.0);
	var1.resize(numConflictTypes-1, 0.0);
	var2.resize(numConflictTypes-1, 0.0);
	
	for (int i=0;i<numTrjFiles1;i++)
	{
		for (int j=0;j<numConflictTypes-1;j++)
		{
			fileConflicts1[i][j]= fileSummaries1[i]->GetConflictTypeCounts()[j+1];
		}
	}
	for (int i=0;i<numTrjFiles2;i++)
	{
		for (int j=0;j<numConflictTypes-1;j++)
		{
			fileConflicts2[i][j]= fileSummaries2[i]->GetConflictTypeCounts()[j+1];	
		}
	}
		    			
	CalcMean(fileConflicts1, mean1);
	CalcMean(fileConflicts2, mean2);
	CalcVar(fileConflicts1,mean1, var1);
	CalcVar(fileConflicts2,mean2, var2);
}


void CFNumTTestRunner::CalcMean(const std::vector<std::vector<int> >& data, 
								std::vector<float>& meanVals)
{
	int nSamples=data.size();
	int nMeasures=data.front().size();
	std::vector<float> sums(nMeasures, 0.0);
	meanVals.resize(nMeasures);
	for (int im=0; im<nMeasures; im++)
	{	
		for (int i=0; i<nSamples; i++)
		{
			sums[im] += data[i][im];
		}
		meanVals[im]=sums[im]/nSamples;		
	}
}

void CFNumTTestRunner::CalcVar(const std::vector<std::vector<int> >& data, 
		const std::vector<float>& meanVals,
		std::vector<float>& varVals)
{
	int nSamples=data.size();
	int nMeasures=data.front().size();
	std::vector<float> sums(nMeasures, 0.0);
	varVals.resize(nMeasures);
	for (int im=0;im<nMeasures;im++)
	{	for (int i=0;i<nSamples;i++)
		{
			sums[im]=sums[im]+(data[i][im]-meanVals[im])*(data[i][im]-meanVals[im]);
		}
		varVals[im]=sums[im]/(nSamples-1);		
	}
}

TTable::TTable()
	: m_TCritical(0)
	, m_TTableFile("tables\\ttable.txt")
{
	ReadTTableFile(m_TTableFile);    
}

TTable::TTable(const std::string& sigLevel, int degreeFreedom)
	: m_TCritical(0)
	, m_TTableFile("tables\\ttable.txt")
{		
	int colIdx = 0;
	if (sigLevel=="0.25")
		colIdx=1;
	else if (sigLevel=="0.20")
		colIdx=2;
	else if (sigLevel=="0.15")
		colIdx=3;
	else if (sigLevel=="0.10")
		colIdx=4;
	else if (sigLevel=="0.05")
		colIdx=5;
	else if (sigLevel=="0.025")
		colIdx=6;
	else if (sigLevel=="0.02")
		colIdx=7;
	else if (sigLevel=="0.01")
		colIdx=8;
	else if (sigLevel=="0.005")
		colIdx=9;
	else if (sigLevel=="0.0025")
		colIdx=10;
	else if (sigLevel=="0.001")
		colIdx=11;
	else if (sigLevel=="0.0005")
		colIdx=12;
	ReadTCritical(colIdx,degreeFreedom);		
}

float TTable::ReadTCritical(int colIdx,int degreeFreedom)
{		
	ReadTTableFile(m_TTableFile);
	int rowIdx = 0;
	if (degreeFreedom<31)
	{
		rowIdx=degreeFreedom;
		m_TCritical=m_Datas[rowIdx][colIdx];			
		return m_TCritical;
	}			
	else if(degreeFreedom>=31&degreeFreedom<61)
	{
		for(int i=1;i<3;i++)
		{
			int lowerDegree=30+i*10;
			int higherDegree=40+i*10;
			if (degreeFreedom<=higherDegree & degreeFreedom>lowerDegree)
			{
				rowIdx=(int)((lowerDegree-30)/10)+30;
				float tLower=m_Datas[rowIdx+1][colIdx];
				float tHigher=m_Datas[rowIdx][colIdx];
				m_TCritical=tLower+((tHigher-tLower)*float(higherDegree-degreeFreedom))/10.0;					
			}				
		}
		return m_TCritical;
	}
	else if(degreeFreedom>=61 & degreeFreedom<101)
	{
		for(int i=0;i<2;i++)
		{
			int lowerDegree=60+i*20;
			int higherDegree=80+i*20;
			if (degreeFreedom<=higherDegree & degreeFreedom>lowerDegree)
			{
				rowIdx=(int)((lowerDegree-60)/20)+33;
				float t_lower=m_Datas[rowIdx+1][colIdx];
				float t_higher=m_Datas[rowIdx][colIdx];
				m_TCritical=t_lower+((t_higher-t_lower)*float(higherDegree-degreeFreedom))/20.0;					
			}				
		}
		return m_TCritical;
	}
	else
	{
		m_TCritical=m_Datas[35][colIdx];
		return m_TCritical;
	}		
}

void TTable::ReadTTableFile(const std::string& tTFile)
{
	std::ifstream inFile(tTFile);
	std::string line; 
	while ( std::getline(inFile, line) )
	{
		std::vector<float> lineData;
		std::stringstream ss(line);
		std::string field;
		while (std::getline(ss, field, ' '))
		{
			if (field == "df")
			{
				lineData.push_back(0);
			} else if (field == "INF." || field == "inf.")
			{
				lineData.push_back(std::numeric_limits<float>::infinity());
			} else if (IsValidNumber(field))
			{
				try 
				{ 
					float val = std::stof(field);
					lineData.push_back(val);
				} catch (const std::invalid_argument& e)
				{
					lineData.push_back(0);
				} 
			} else
			{
				lineData.push_back(0);
			}
		}
		m_Datas.push_back(lineData);
	}
	inFile.close();
}

bool TTable::IsValidNumber(const std::string& str)
{
	if (str.empty())
		return false;
	for (size_t i = 0; i < str.length(); ++i)
	{
		if (!isdigit(str[i]) && str[i]!='.')
			return false;
	}
	return true;
}

FTable::FTable(const std::string& sig_level, int degreeone,int degreetwo)
	: m_FCritical(0.0)
{
	if (sig_level=="0.01")
	{
		m_FTFile="tables\\ftable001.txt";
	}
	else if (sig_level=="0.025")
	{
		m_FTFile="tables\\ftable0025.txt";
	}
	else if (sig_level=="0.05")
	{
		m_FTFile="tables\\ftable005.txt";
	}
	else if (sig_level=="0.1")
	{
		m_FTFile="tables\\ftable01.txt";
	}
	else 
	{
		m_FTFile="";
	}
	if (!m_FTFile.empty())
	{
		ReadFTableFile(m_FTFile);  
		ReadFCritical(degreeone,degreetwo);
	}		
}
	
float FTable::ReadFCritical(int degree1,int degree2)
{		
	if(degree1<=120 && degree2<=120)
	{
		int iCol=1;
		int lowerCol=0;
		int higherCol=0;
		while(degree1>=GetInt(m_Datas[0][iCol]))
		{
			lowerCol=iCol;
			higherCol=iCol+1;
			iCol++;
		}
		int lowDegree1=GetInt(m_Datas[0][lowerCol]);
		int highDegree1=GetInt(m_Datas[0][higherCol]);
					
		int iRow=1;
		int lowerRow=0;
		int higherRow=0;
		while(degree2>=GetInt(m_Datas[iRow][0]))
		{
			lowerRow=iRow;
			higherRow=iRow+1;
			iRow++;
		}
		int lowDegree2=GetInt(m_Datas[lowerRow][0]);
		int highDegree2=GetInt(m_Datas[higherRow][0]);
			
		float value1=(m_Datas[lowerRow][lowerCol]);
		float value2=(m_Datas[lowerRow][higherCol]);						
		float value3=(m_Datas[higherRow][lowerCol]);
		float value4=(m_Datas[higherRow][higherCol]);
			
		float midvalue1=value2+(value1-value2)*(((float)(highDegree1-degree1))/((float)(highDegree1-lowDegree1)));
		float midvalue2=value4+(value3-value4)*(((float)(highDegree1-degree1))/((float)(highDegree1-lowDegree1)));
			
		m_FCritical=midvalue2+(midvalue1-midvalue2)*(((float)(highDegree2-degree2))/((float)(highDegree2-lowDegree2)));
		return m_FCritical;
	}
	else if (degree1<=120 && degree2>120)
	{
		int iCol=1;
		int lowerCol=0;
		int higherCol=0;
		while(degree1>=GetInt(m_Datas[0][iCol]))
		{
			lowerCol=iCol;
			higherCol=iCol+1;
			iCol++;
		}
		int lowDegree=GetInt(m_Datas[0][lowerCol]);
		int highDegree=GetInt(m_Datas[0][higherCol]);
			
		float value1=(m_Datas[34][lowerCol]);
		float value2=(m_Datas[34][higherCol]);						
		
		m_FCritical=value2+(value1-value2)*(((float)(highDegree-degree1))/((float)(highDegree-lowDegree)));
		return m_FCritical;
	}
	else if (degree1>120 && degree2<=120)
	{
		int iRow=1;
		int lowerRow=0;
		int higherRow=0;
		while(degree2>=GetInt(m_Datas[iRow][0]))
		{
			lowerRow=iRow;
			higherRow=iRow+1;
			iRow++;
		}
		int lowDegree=GetInt(m_Datas[lowerRow][0]);
		int highDegree=GetInt(m_Datas[higherRow][0]);
			
		float value1=(m_Datas[lowerRow][19]);
		float value2=(m_Datas[higherRow][19]);						
		
		m_FCritical=value2+(value1-value2)*(((float)(highDegree-degree1))/((float)(highDegree-lowDegree)));
		return m_FCritical;
	}
	else
	{
		m_FCritical=(m_Datas[34][19]);
		return m_FCritical;
	}		
}

void FTable::ReadFTableFile(const std::string& aFile) 
{       
    std::ifstream inFile(aFile);
    std::string line; 
    std::string continuedline;
	while ( std::getline(inFile, line) )
    {
		std::vector<float> lineData;
		std::stringstream ss(line);
		std::string field;
		int k = 0;
		while (std::getline(ss, field, ' '))
		{
			if (field == "df2/df1")
			{
				lineData.push_back(0);
			} else if (field == "INF" || field == "inf")
			{
				lineData.push_back(std::numeric_limits<float>::infinity());
			} else
			{
				try 
				{ 
					float val = std::stof(field);
					lineData.push_back(val);
				} catch (const std::invalid_argument& e)
				{
					lineData.push_back(0);
				} 
			}
			k++;
		}
		if (k < 20)
		{
			std::getline(inFile, continuedline);
			std::stringstream css(continuedline);
			while (std::getline(css, field, ' '))
			{
				if (field == "df2/df1")
				{
					lineData.push_back(0);
				} else if (field == "INF" || field == "inf")
				{
					lineData.push_back(std::numeric_limits<float>::infinity());
				} else
				{
					try 
					{ 
						float val = std::stof(field);
						lineData.push_back(val);
					} catch (const std::invalid_argument& e)
					{
						lineData.push_back(0);
					} 
				}
			}
		}
		m_Datas.push_back(lineData);	      	  
    }
 }
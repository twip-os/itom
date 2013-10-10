#include <iostream>

#include "../../Common/sharedStructures.h"

//opencv
#pragma warning( disable : 4996 ) //C:\OpenCV2.3\build\include\opencv2/flann/logger.h(70): warning C4996: 'fopen': This function or variable may be unsafe. Consider using fopen_s instead.
#pragma once
#include "opencv/cv.h"
#include "../../DataObject/dataobj.h"
#include "gtest/gtest.h"
#include "commonChannel.h"


/*! \class at_func_test
	\brief at() methods test for data objects with real data types

	This test class checks functionality of different implementations of at() function on data objects of different sizes and different real data types.
*/
template <typename _Tp> class seekMat_calcNumMats_test : public ::testing::Test 
	{ 
public:
	
	virtual void SetUp(void)
	{
		dObj1 = ito::DataObject();
		dObj2 = ito::DataObject(10,10,ito::getDataType( (const _Tp *) NULL ));
		dObj3 = ito::DataObject(5,10,10,ito::getDataType( (const _Tp *) NULL ));
		size_t *temp_size = new size_t[5];
		temp_size[0] = 3;
		temp_size[1] = 4;
		temp_size[2] = 2;
		temp_size[3] = 10;
		temp_size[4] = 10;
		dObj5 = ito::DataObject(5,temp_size,ito::getDataType( (const _Tp *) NULL ));
	};
 
	virtual void TearDown(void) {};

	typedef _Tp valueType;	
	ito::DataObject dObj1;
	ito::DataObject dObj2;
	ito::DataObject dObj3;
	ito::DataObject dObj4;
	ito::DataObject dObj5;
	ito::DataObject dObj6;
	};
	
TYPED_TEST_CASE(seekMat_calcNumMats_test, ItomDataAllTypes /*ItomRealDataTypes*/);

//seekMat_calNumMats_Test
/*!
	This test checks the functionality of seekMat() and calNumMats() methods.
*/
TYPED_TEST(seekMat_calcNumMats_test, seek_calc_test1)
{
	int i ;
	ito::Range *ranges1 = new ito::Range[3];				//!< declaring range to assign sub matrix from dObj3 into dObj4.
	ranges1[1]=ito::Range::all();
	ranges1[2]=ito::Range::all();
	ranges1[0]=ito::Range(2,4);

	dObj4= dObj3.at(ranges1);

	ito::Range *ranges2 = new ito::Range[5];				//!< declaring range to assign sub matrix from dObj5 into dObj6.
	ranges2[0]=ito::Range(1,3);
	ranges2[1]=ito::Range(2,3);
	ranges2[2]=ito::Range::all();
	ranges2[3]=ito::Range::all();
	ranges2[4]=ito::Range::all();

	dObj6= dObj5.at(ranges2);								//!< dObj6 = dObj5(1:3,2:3,:,:,:) is done using at() method.

	for(i=0;i<5;i++) EXPECT_EQ(0,dObj1.seekMat(i));			//!< Test for seekMat() function for one dimensional DataObject of Real DataTypes. 		
	for(i=0;i<5;i++) EXPECT_EQ(0, dObj2.seekMat(i));		//!< Test for seekMat() function for two dimensional DataObject of Real DataTypes.
	int test_str1[] = {0,1,2,3,4,0};
	for(i=0;i<6;i++) EXPECT_EQ(test_str1[i],dObj3.seekMat(i));	  //!< Test for seekMat() function for 3 dimensional DataObject of Real DataTypes.
	int test_str2[] = {2,3,0,0,0};
	for(i=0;i<5;i++) EXPECT_EQ(test_str2[i],dObj4.seekMat(i));	  //!< Test for seekMat() function for ROI of a previous 3 dimensional DataObject "dObj3".
	int test_str3[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,0,0,0,0,0,0,0};
	for(i=0;i<31;i++) EXPECT_EQ(test_str3[i],dObj5.seekMat(i));	  //!< Test for seekMat() function for 5 dimensional DataObject of Real DataTypes.
	int test_str4[] = {12,13,20,21,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	for(i=0;i<31;i++) EXPECT_EQ(test_str4[i],dObj6.seekMat(i));	  //!< Test for seekMat() function for ROI of a previous 5 dimensional DataObject "dObj5".
	EXPECT_EQ(0,dObj1.calcNumMats());						//!< Test for calcNumMats() function for 1 dimensional DataObject of Real DataTypes.
	EXPECT_EQ(1,dObj2.calcNumMats());						//!< Test for calcNumMats() function for 2 dimensional DataObject of Real DataTypes.
	EXPECT_EQ(5,dObj3.calcNumMats());						//!< Test for calcNumMats() function for 3 dimensional DataObject of Real DataTypes.
	EXPECT_EQ(2,dObj4.calcNumMats());						//!< Test for calcNumMats() function for ROI of a previous 3 dimensional DataObject "dObj3".
	EXPECT_EQ(24,dObj5.calcNumMats());						//!< Test for calcNumMats() function for 1 dimensional DataObject of Real DataTypes.	
	EXPECT_EQ(4,dObj6.calcNumMats());						//!< Test for calcNumMats() function for ROI of a previous 5 dimensional DataObject "dObj5".

	delete[] ranges1;
	delete[] ranges2;
}




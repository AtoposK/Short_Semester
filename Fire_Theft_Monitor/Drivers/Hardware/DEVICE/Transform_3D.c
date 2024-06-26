/*******************************************************************************
* 文档: 3D-transform.c
* 作者: 执念执战
* 		QQ:572314251
*		微信：zhinianzhizhan
*		随梦，随心，随愿，恒执念，为梦执战，执战苍天！
*
* 描述:
*	1.三维图形变换操作
*       2.3D-transform.c包含“变换算法”和“投影算法”
*       
*       3.3D_Rotateapplitation对“变换矩阵算法”和“投影算法”的应用
*       4.rotation_font3D.c实现三维空间中旋转图片（优化算法）
*       5.本3D算法学习自《基于STC单片机“12864液晶显示旋转立方体和图片（优化算法）”实例》
*	6.		作者是Wu LianWei
*	7.本文档为“变换矩阵算法”和“投影算法”
*	8：注意：视点到平面的距离，当屏幕上的立方图转着转着乱了，
		满图画色时试着改改视点到物体的距离FOCAL_DISTANCE、比例变换矩阵中数值的大小及XO,YO 的大小
		要确保图形旋转过程中不会超出边界，不然就会乱码
	9.下面注释中的240 400等数字是之前在240*400的屏幕上移植时写的，自行转换为128   64 等，不影响
	
******* 进行了后期整合，将多个文档整合为四个。//时间：2017/2/5
*******
	10.转载使用时请注明出处
*******************************************************************************/
#include <math.h>
#include <Display_3D.h>

/**********************************************************/
//**函数：MATRIX_copy
//**功能：矩阵拷贝
//**说明：  sourceMAT[4][4]（源矩阵，source matrix），
//				   targetMAT[4][4] (目标矩阵 target matrix)
//					 边学边写
//**作者：  执念执战
///**时间：2015-11-29，21：52  
/***********************************************************/
void MATRIX_copy(float sourceMAT[4][4],float targetMAT[4][4])
{
	uint8_t a,b;
	for(a=0;a<4;a++)
	{
		for(b=0;b<4;b++)
		{
			targetMAT[a][b]=sourceMAT[a][b];
		}
	}
}
/**********************************************************/
///**函数：MATRIX_multiply
///**功能：矩阵相乘
///**说明： MAT1[4][4]（矩阵1），MAT2[4][4]（矩阵2），
//					newMAT[4][4] (结果矩阵 )
//					 边学边写
//**作者：  执念执战
///**时间：2015-11-29
/***********************************************************/
void MATRIX_multiply(float MAT1[4][4],float MAT2[4][4],float newMAT[4][4])
{
	uint8_t a,b;
	for(a=0;a<4;a++)
		for(b=0;b<4;b++)
			newMAT[a][b]=MAT1[a][0]*MAT2[0][b]+MAT1[a][1]*MAT2[1][b]+MAT1[a][2]*MAT2[2][b]+MAT1[a][3]*MAT2[3][b];
				
}
///**********************************************************/
///**函数: vector_matrix_MULTIPLY
///**功能：矢量与矩阵相乘
///**说明： Source(源矢量坐标)  MAT[][]（变换坐标）
//					 边学边写
//**作者：  执念执战
///**时间：2015-11-29
///***********************************************************/
_3Dzuobiao vector_matrix_MULTIPLY(_3Dzuobiao Source,float MAT[4][4])
{
	_3Dzuobiao  Result;
	Result.x=Source.x*MAT[0][0]+Source.y*MAT[1][0]+Source.z*MAT[2][0]+MAT[3][0];
	Result.y=Source.x*MAT[0][1]+Source.y*MAT[1][1]+Source.z*MAT[2][1]+MAT[3][1];
	Result.z=Source.x*MAT[0][2]+Source.y*MAT[1][2]+Source.z*MAT[2][2]+MAT[3][2];
	return Result;
}
///**********************************************************/
///**函数: structure_3D
///**功能：构造单位矩阵
///**说明： 将一个数组构造成单位矩阵（对角线上全为1，其他为0）
//					 边学边写
//**作者：  执念执战
///**时间：2015-11-29
///***********************************************************/
void structure_3D(float MAT[4][4])
{
    MAT[0][0]=1.0;MAT[0][1]=0.0;MAT[0][2]=0.0;MAT[0][3]=0.0;// 1 0 0 0 
    MAT[1][0]=0.0;MAT[1][1]=1.0;MAT[1][2]=0.0;MAT[1][3]=0.0;// 0 1 0 0
    MAT[2][0]=0.0;MAT[2][1]=0.0;MAT[2][2]=1.0;MAT[2][3]=0.0;// 0 0 1 0
    MAT[3][0]=0.0;MAT[3][1]=0.0;MAT[3][2]=0.0;MAT[3][3]=1.0;// 0 0 0 1
}
///**********************************************************/
///**函数: Translate3D
///**功能：平移变换矩阵
///**说明： 1:tx,ty,tz 为平移参数
//					2：当为边长的一半的倒数时，图像绕着自己旋转，即原点在中心
//					
//					 边学边写
//**作者：  执念执战
///**时间：2015-11-29
///***********************************************************/
void Translate3D(float MAT[4][4],int16_t tx,int16_t ty,int16_t tz)
{
	float lMAT[4][4];
	float tMAT[4][4];
//	tMAT[0][0]=1;  tMAT[0][1]=0;  tMAT[0][2]=0;  tMAT[0][3]=tx;	//	1 0 0 tx
//	tMAT[1][0]=0;  tMAT[1][1]=1;  tMAT[1][2]=0;  tMAT[1][3]=ty;	//	0 1 0 ty
//    tMAT[2][0]=0;  tMAT[2][1]=0;  tMAT[2][2]=1;  tMAT[2][3]=tz;	//	0 0 1 tz
//    tMAT[3][0]=0; tMAT[3][1]=0; tMAT[3][2]=0; tMAT[3][3]=1;		//	0 0 0 1
//	MATRIX_multiply(MAT,tMAT,lMAT);//相乘
//	MATRIX_copy(lMAT,MAT);				//复制
	tMAT[0][0]=1;  tMAT[0][1]=0;  tMAT[0][2]=0;  tMAT[0][3]=0;	//	1 0 0 tx
	tMAT[1][0]=0;  tMAT[1][1]=1;  tMAT[1][2]=0;  tMAT[1][3]=0;	//	0 1 0 ty
	tMAT[2][0]=0;  tMAT[2][1]=0;  tMAT[2][2]=1;  tMAT[2][3]=0;		//	0 0 1 tz
	tMAT[3][0]=tx; tMAT[3][1]=ty; tMAT[3][2]=tz; tMAT[3][3]=1;		//	0 0 0 1
	MATRIX_multiply(MAT,tMAT,lMAT);//相乘
	MATRIX_copy(lMAT,MAT);				//复制
		
}
///**********************************************************/
///**函数: Scale_3D
///**功能：比例(scale)变换矩阵
///**说明： 1：相对于原点上的比例变换
//					2：sx,sy,sz 是对应轴上的缩放量
//					3:矩阵根据三个比例进行比例变换
//					 边学边写
//**作者：  执念执战
///**时间：2015-11-30,7:45
///***********************************************************/
void  Scale_3D(float MAT[4][4],float sx,float sy,float sz)
{
	float lMAT[4][4];
	float tMAT[4][4];
	tMAT[0][0]=sx;  tMAT[0][1]=0;  tMAT[0][2]=0;  tMAT[0][3]=0;	//	sx0 0 0 
	tMAT[1][0]=0;   tMAT[1][1]=sy; tMAT[1][2]=0;  tMAT[1][3]=0;	//	0 sy0 0
	tMAT[2][0]=0;   tMAT[2][1]=0;  tMAT[2][2]=sz; tMAT[2][3]=0;	//	0 0 sz0
	tMAT[3][0]=0;   tMAT[3][1]=0;  tMAT[3][2]=0;  tMAT[3][3]=1;	//	0 0 0 1
	MATRIX_multiply(MAT,tMAT,lMAT);	//相乘
	MATRIX_copy(lMAT,MAT);					//复制
}
///**********************************************************/
///**函数: Rotate_3D
///**功能：旋转变换矩阵
///**说明： 1：绕z轴旋转az度角
//					2：az为旋转的角度量，赋正值为顺时针转
//					3:矩阵MAT根据AZ度角进行旋转变换
//					 边学边写
//**作者：  执念执战
///**时间：2015-11-30,7:45
///***********************************************************/

void Rotate_3D(float MAT[4][4],float ax,float ay,float az)
{
	float MAT1[4][4];
	float MAT2[4][4];
	float MATx[4][4];
	float MATy[4][4];
	float MATz[4][4];
	
	ax=(3.1415926*ax)/180.0;	//角度转换为弧度量
	ay=(3.1415926*ay)/180.0;
	az=(3.1415926*az)/180.0;
	/*****************************绕x轴旋转********************************************/
	
	MATx[0][0]=1;	MATx[0][1]=0;		MATx[0][2]=0;		MATx[0][3]=0;//1  0		0	   0
	MATx[1][0]=0;	MATx[1][1]=cos(ax);	MATx[1][2]=-sin(ax);MATx[1][3]=0;//0  cos(ax)	-sin(ax)   0
	MATx[2][0]=0;	MATx[2][1]=sin(ax); MATx[2][2]=cos(ax);	MATx[2][3]=0;//0  sin(ax)	cos(ax)    0	
	MATx[3][0]=0;	MATx[3][1]=0;		MATx[3][2]=0;		MATx[3][3]=1;//0  0		 0	   1
	
	/*****************************绕y轴旋转********************************************/
	MATy[0][0]=cos(ay);	MATy[0][1]=0;		MATy[0][2]=sin(ay);	MATy[0][3]=0;//cos(ay)	0	sin(ay)		0
	MATy[1][0]=0;		MATy[1][1]=1;		MATy[1][2]=0;		MATy[1][3]=0;//	0	1	0		0
	MATy[2][0]=-sin(ay);MATy[2][1]=0;		MATy[2][2]=cos(ay);	MATy[2][3]=0;//	-sin(ay)	0		cos(ay) 0
	MATy[3][0]=0;		MATy[3][1]=0;		MATy[3][2]=0;		MATy[3][3]=1;// 0 	0	0		1
	
	/*****************************绕z轴旋转********************************************/
	MATz[0][0]=cos(az);	MATz[0][1]=-sin(az);MATz[0][2]=0;	MATz[0][3]=0;//cos(az)	-sin(az)	0		0
	MATz[1][0]=sin(az);	MATz[1][1]=cos(az);	MATz[1][2]=0;	MATz[1][3]=0;//	sin(az)	cos(az)		0		0
	MATz[2][0]=0;		MATz[2][1]=0;		MATz[2][2]=1;	MATz[2][3]=0;//	0	  0		1		0
	MATz[3][0]=0;		MATz[3][1]=0;		MATz[3][2]=0;	MATz[3][3]=1;// 0 	  0		0		1
	
	MATRIX_multiply(MAT,MATx,MAT1);	
	MATRIX_multiply(MAT1,MATy,MAT2);	
	MATRIX_multiply(MAT2,MATz,MAT);	
			
}
/********************************************************************************/
/*------------------------------------------------------------------------------*/
/*			投影算法	三维转二维的相关函数											*/
/*------------------------------------------------------------------------------*/
/********************************************************************************/

///**********************************************************/
///**函数: Rotate_3D
///**功能：正射投影（Orthographic projection）
///**说明：将三维坐标中的x,y坐标赋值给二维坐标的x,y

//					 边学边写
//**作者：  执念执战
///**时间：2015-11-30
///***********************************************************/
_2Dzuobiao OrtProject(_3Dzuobiao Space)
{
	_2Dzuobiao	Screen;
	Screen.x=(int16_t)Space.x;
	Screen.y=(int16_t)Space.y;
	
	return Screen;
}

/**************************************************/
/*基于透视投影的基本模型*/
#define FOCAL_DISTANCE 512	//视点到平面的距离，当屏幕上的立方图转着转着乱了
				//	，满图画色时试着改改这个,和比例变换矩阵中数值的大小及XO,YO 的大小
				//不要太小或太大，

//int16_t  XO;
//int16_t  YO;

//XO=120,YO=120;
///**********************************************************/
///**函数: PerProject
///**功能：透视投影（Perspective projection）
///**说明：XO,YO为投影后的图形中心的屏幕坐标

//					 边学边写
//**作者：  执念执战
///**时间：2015-11-30
///***********************************************************/
_2Dzuobiao	PerProject(_3Dzuobiao Space,int16_t  XO,int16_t  YO)
{
	_2Dzuobiao Screen;
	if (Space.z==0)Space.z=1;		//被除数不能为零
	Screen.x=(int16_t)(FOCAL_DISTANCE*Space.x /(Space.z+FOCAL_DISTANCE)+XO);
	Screen.y=(int16_t)(FOCAL_DISTANCE*Space.y /(Space.z+FOCAL_DISTANCE)+YO);
	return Screen;
}
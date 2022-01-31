// 定义一些模型中用于保存和读取时使用的类型和code
#ifndef _QX_STG_MODEL_DEF_H_
#define _QX_STG_MODEL_DEF_H_

//model中块类型定义
#define KSTG_BLOCK_DOC_HEADER		0x01	 //doc文件的文件头
#define KSTG_BLOCK_DOC_GLOBAL		0x10	 //doc文件的全局数据
// model数据块类型
#define KSTG_BLOCK_MODEL			0x0100	//模块数据
#define KSTG_BLOCK_MODEL_END		0x0101	//模块数据

#define KSTG_BLOCK_LAYERS			0x0102	//图层数据起始
#define KSTG_BLOCK_LAYERS_END		0x0103	//图层数据终止
#define KSTG_BLOCK_LAYER			0x0104	//图层对象
#define KSTG_BLOCK_LAYER_END		0x0105	//图层对象
//
#define KSTG_BLOCK_ENTITY_TYPE		0x0106  //entity类型
#define KSTG_BLOCK_ENTITY			0x0107  //entity
#define KSTG_BLOCK_ENTITY_ATTRIB   0x0108  //entity属性块

#define KSTG_BLOCK_BASEPLANES_BEGIN 	0x0110  //基准面数据
#define KSTG_BLOCK_BASEPLANES_END		0x0111  //基准面数据
#define KSTG_BLOCK_BASEPLANE			0x0112  //基准面数据

//值对的code的固定值,如果是该类型，尽量使用该值
//尽量大一些，留下小值方便内部使用
#define KSTG_CV_COMMON_CODE_BASE  200
#define KSTG_CV_CODE_BLOCK_TAG		(KSTG_CV_COMMON_CODE_BASE+1)  //块的唯一标识值
#define KSTG_CV_CODE_TYPE			(KSTG_CV_COMMON_CODE_BASE+2)   //类型
#define KSTG_CV_CODE_NAME			(KSTG_CV_COMMON_CODE_BASE+3)   //名称
#define KSTG_CV_CODE_HANDLE		(KSTG_CV_COMMON_CODE_BASE+4)   //句柄code
#define KSTG_CV_CODE_SHAPE			(KSTG_CV_COMMON_CODE_BASE+5)   //shape的数据
#define KSTG_CV_CODE_VISIBLE		(KSTG_CV_COMMON_CODE_BASE+6)   //是否可见
//
#define KSTG_CV_CODE_COLOR			(KSTG_CV_COMMON_CODE_BASE+7)   //颜色


#endif
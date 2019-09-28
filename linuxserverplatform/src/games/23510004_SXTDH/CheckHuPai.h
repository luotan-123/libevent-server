#ifndef CHECKHUPAI_H
#define CHECKHUPAI_H
#include "GameDataEx.h"
#include "CLogicBase.h"
#include "mj.h"

///糊牌检测类
class CheckHuPai
{
public:
	///糊牌数结构数组
	PingHuStruct	m_tHuPaiStruct;
	///糊牌数结构数组
	PingHuStruct	m_tTempHupaistruct;
	///糊牌检测临时数据
	CheckHuStruct	m_tHuTempData;
	///糊牌检测临时数据(无财神)
	CheckHuStruct	m_tNoJingHuTempData;
	///当前检测玩家的位置
	BYTE            m_byStation;
	///玩家是否自摸胡牌
	bool            m_bZimo;
	///精牌总数
	BYTE			m_byJingNum;
	///糊牌类型
	BYTE			m_byHuType[MAX_HUPAI_TYPE];
	//当前最大番数值
	int				m_iMaxFan;
	//最后自摸或点炮的牌
	BYTE			m_byLastPai;
	//放炮玩家位置
	BYTE			m_byBeStation;
	//手牌不超过20张，0为结束标志
	BYTE			m_byArHandPai[HAND_CARD_NUM];
	///组牌数据
	BYTE		   m_byShunNum;			//顺子的个数
	BYTE           m_byShunData[MAX_CPG_NUM];//顺子的数据
	BYTE		   m_byKeziNum;			//刻子的个数
	BYTE           m_byKeziData[MAX_CPG_NUM];//刻子的数据
	BYTE		   m_byGangNum;			//杠牌的个数
	BYTE           m_byGangData[MAX_CPG_NUM][2];//杠牌的数据
	BYTE		   m_byJiang;			//将牌数据
	BYTE		   m_byChiNum;			//吃牌的个数
	BYTE		   m_byPengNum;			//碰牌的个数
	BYTE           m_byTypeFans[MAX_HUPAI_TYPE];//类型番数	

	BYTE		   m_byHandShunNum;			//手中顺子的个数，不包括吃
	BYTE           m_byHandShunData[MAX_CPG_NUM];//顺子的数据，不包括吃
public:
	CheckHuPai(void);
	~CheckHuPai(void);
	///初始化数据
	virtual void InitData();

	//////////////////////////////////////////////////////////////////////////平胡
	//获取胡牌类型
	///平胡：获取最大的组合牌型
	virtual bool GetHuPaiTypes(GameDataEx *pGameData, bool zimo);

	///检测能否听牌
	virtual bool CanTing(GameDataEx *pGameData, BYTE station, BYTE CanOut[]);

	///检测能否吃听牌：吃后听牌
	virtual bool ChiTing(GameDataEx *pGameData, BYTE station, BYTE CanOut[], BYTE pai[][3]);

	///检测杠后能否保存听牌状态
	virtual bool GangTing(GameDataEx *pGameData, BYTE station, tagCPGNotify &cpg, BYTE ps = 255, bool CanTing[] = NULL);

	//检测糊牌
	virtual bool CanHu(GameDataEx *pGameData, BYTE station, BYTE lastpai, bool zimo);

	///将吃碰杠数据加入到糊牌结构中
	virtual void AddGCPData(TCPGStruct gcpData[], PingHuStruct * tempstruct);

	///转换手牌数据的存储方式，为了方便糊牌检测
	virtual void ChangeHandPaiData(GameDataEx *pGameData, BYTE handpai[]);

	///转换手牌数据的存储方式，为了方便糊牌检测
	virtual void ChangeHandPaiDataEx(GameDataEx *pGameData, BYTE handpai[]);

	///添加一张牌到临时数组中
	virtual void AddAPaiToArr(BYTE pai);

	///平糊检测
	virtual bool CheckPingHu(GameDataEx *pGameData);

	///设置精牌
	virtual void SetJingPai();

	///恢复精牌
	virtual void ReSetJingPai();

	///平糊组牌
	virtual bool MakePingHu(GameDataEx *pGameData, CheckHuStruct &PaiData, PingHuStruct &m_tHuPaiStruct, int iJingnum = 0);

	///恢复牌数,把减掉的牌还原
	virtual void ReSetPaiNum(BYTE num[], CheckHuStruct &TempPai);

	///检测连牌是否同一花色
	virtual bool CheckTongSe(BYTE pai1, BYTE pai2);

	///检测连牌是否连张
	virtual bool CheckLianPai(GameDataEx *pGameData, BYTE pai1, BYTE pai2, BYTE num = 1);

	///描述：清除组牌数据
	virtual void ClearPingHuData();

	///检测平糊数据组合：将牌，吃，碰杠，顺子，暗刻等
	virtual void CheckPingHuData(GameDataEx *pGameData);

	///添加一种糊牌类型
	virtual void SetAHupaiType(BYTE type, BYTE hupai[]);

	///撤销一种糊牌类型
	virtual void ReSetAHupaiType(BYTE type, BYTE hupai[]);

	///描述：清除所有糊牌类型
	virtual void ClearHupaiType(BYTE hupai[]);

	///检测是否存在某种糊牌类型
	virtual bool CheckHupaiType(BYTE type, BYTE hupai[]);

	/////////////////详细糊牌类型检测///////////////////////////////////////
	virtual bool CheckTianHu(GameDataEx *pGameData);//天糊
	virtual bool CheckDiHu(GameDataEx *pGameData);//地糊	

	//88番牌型（5种）
	virtual bool CheckDaSiXi();//大四喜
	virtual bool CheckDaSanYuan();//大三元
	virtual bool CheckSiGang();//四杠（十八罗汉）
	virtual bool CheckJiuLianBaoDeng(GameDataEx *pGameData);//九莲宝灯	
	virtual bool CheckLianQiDui(GameDataEx *pGameData);//连七对（七对的特殊）

	//64番牌型（5种）
	virtual bool CheckXiaoSiXi();//小四喜
	virtual bool CheckXiaoSanYuan();//小三元	
	virtual bool CheckZiYiSe(GameDataEx *pGameData);//字一色	
	virtual bool CheckSiAnKe(GameDataEx *pGameData);//四暗刻
	virtual bool CheckYiSeShuangLongHui();//一色双龙会	

	//48番牌型（2种）
	virtual bool CheckYiSeSiTongShun();//一色四同顺
	virtual bool CheckYiSeSiJieGao();//一色四节高	

	//32番牌型（3种）
	virtual bool CheckYiSeSiBuGao();//一色四步高	
	virtual bool CheckHunYaoJiu(GameDataEx *pGameData);//混幺九	
	virtual bool CheckSanGang();//三杠

	//24番牌型（4种）
	virtual bool CheckQiDui(GameDataEx *pGameData);//七对
	virtual bool CheckLongQiDui(GameDataEx *pGameData);//龙七对
	virtual bool CheckQingYiSeQiDui(GameDataEx *pGameData);//清一色七对
	virtual bool CheckQingYiSe(GameDataEx *pGameData);//清一色	 
	virtual bool CheckQingYiSeYiTiaoLong(GameDataEx *pGameData);//清一色一条龙	 
	virtual bool CheckYiSeSanTongShun();//一色三同顺	
	virtual bool CheckYiSeSanJieGao();//一色三节高	

	//16番牌型（3种）
	virtual bool CheckQingLong();//清龙	
	virtual bool CheckYiSeSanBuGao();//一色三步高	
	virtual bool CheckSanAnKe();//三暗刻

	//12番牌型（3种）
	virtual bool CheckDaYuWu(GameDataEx *pGameData);//大于五	
	virtual bool CheckXiaoYuWu(GameDataEx *pGameData);//小于五	
	virtual bool CheckSanFengKe();//三风刻	

	//8番牌型（4种）
	virtual bool CheckMiaoShouHuiChun(GameDataEx *pGameData);//妙手回春	
	virtual bool CheckHaiDiLaoYue(GameDataEx *pGameData);//海底捞月	

	virtual bool CheckGangKai(GameDataEx *pGameData);//杠上花
	virtual bool CheckQiangGangHe(GameDataEx *pGameData);//抢杠和	

	//6番牌型（5种）
	virtual bool CheckPengPengHu(GameDataEx *pGameData);//碰碰和	
	virtual bool CheckHunYiSe(GameDataEx *pGameData);//混一色	
	virtual bool CheckQuanQiuRen(GameDataEx *pGameData);//全求人	
	virtual bool CheckShuangAnKe();//双暗杠	
	virtual bool CheckShuangJianKe();//双箭刻	

	//4番牌型（4种）
	virtual bool CheckQuanDaiYao(GameDataEx *pGameData);//全带幺	
	virtual bool CheckBuQiuRen(GameDataEx *pGameData);//不求人	
	virtual bool CheckShuangMingGang();//双明杠	
	virtual bool CheckHuJueZhang();//和绝张		

	//2番牌型（7种）
	virtual bool CheckJianKe();//箭刻
	virtual bool CheckMenQianQing(GameDataEx *pGameData);//门前清	
	virtual bool CheckSiGuiYi(GameDataEx *pGameData);//四归一	
	virtual bool CheckShuangAnGang();//双暗刻	
	virtual bool CheckAnGang();//暗杠	
	virtual bool CheckDuanYao(GameDataEx *pGameData);//断幺	

	//1番牌型（10种）
	virtual bool CheckYiBanGao();//一般高	
	virtual bool CheckLianLiu();//连六	
	virtual bool CheckLaoShaoFu();//老少副	
	virtual bool CheckYaoJiuKe();//幺九刻	
	virtual bool CheckMingGang();//明杠	
	virtual bool CheckBianZhang(GameDataEx *pGameData);//边张	
	virtual bool CheckKanZhang(GameDataEx *pGameData);//坎张	
	virtual bool CheckDanDiaoJiang(GameDataEx *pGameData);//单钓将	

	//////////////////以下牌型在雀神中不算番////////////////////////////////
	virtual bool CheckGangHouPao(GameDataEx *pGameData);//杠后炮
	virtual bool CheckXiXiangFeng();//喜相逢
	virtual bool CheckQueYiMen();//缺一门	
	virtual bool CheckWuZi();//无字
	virtual bool CheckQuanFengKe(GameDataEx *pGameData);//圈风刻	
	virtual bool CheckMenFengKe();//门风刻	
	virtual bool CheckShuangTongKe();//双同刻	
	virtual int  CheckYiSe(GameDataEx *pGameData);//一色牌检测（检测手牌和吃碰杠牌，哪里都适用）：0不是一色牌，1混一色，2清一色，	
	virtual bool CheckSanSeSanBuGao();//三色三步高	
	virtual bool CheckWuMenQi(GameDataEx *pGameData);//五门齐	
	virtual bool CheckHuaLong();//花龙	
	virtual bool CheckTuiBuDao(GameDataEx *pGameData);//推不倒	
	virtual bool CheckSanSeSanTongShun();//三色三同顺	
	virtual bool CheckSanSeSanJieGao();//三色三节高	
	virtual bool CheckWuFanHe();//无番和	
	virtual bool CheckSanSeShuangLongHui();//三色双龙会	
	virtual bool CheckQuanDaiWu(GameDataEx *pGameData);	//全带五
	virtual bool CheckSanTongKe();//三同刻		
	virtual bool CheckQuanShuangKe();//全双刻	
	virtual bool CheckQuanDa(GameDataEx *pGameData);//全大	
	virtual bool CheckQuanZhong(GameDataEx *pGameData);//全中	
	virtual bool CheckQuanXiao(GameDataEx *pGameData);//全小		
	virtual bool CheckQingYaoJiu(GameDataEx *pGameData);//清幺九	
	virtual bool CheckLvYiSe(GameDataEx *pGameData);//绿一色	

	/////////////////////////特殊牌型////////////////////////////////////////////////////////////
	virtual bool CheckQuanBuKao(GameDataEx *pGameData);	//全不靠（即十三烂）

	virtual bool CheckZuHeLong(GameDataEx *pGameData);//组合龙	

	virtual bool CheckQiXingBuKao(GameDataEx *pGameData);//七星不靠（七星十三烂）	

	virtual bool CheckShiSanYao(GameDataEx *pGameData);//十三幺	

	virtual bool CheckDaPiao(GameDataEx *pGameData);//大飘(源自江西麻将)	

	virtual bool CheckJiangDui(GameDataEx *pGameData);//检测将对（258碰碰胡）

	virtual bool CheckYiJiuHu(GameDataEx *pGameData);//混幺九

	virtual bool CheckQingYiJiuHu(GameDataEx *pGameData);//清幺九

	bool QiangQiangHu258(GameDataEx *pGameData);//将一色：：手上全部是258时所胡的牌

	virtual bool CheckYiTiaoLong();//一条龙，当胡牌时手中有不同花色的123456789顺子称为一条龙
};
#endif
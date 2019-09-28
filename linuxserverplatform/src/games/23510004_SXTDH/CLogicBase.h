#ifndef CLOGICBASE
#define CLOGICBASE
//基础逻辑类逻辑类 
//处理：数据基本操作，插入删除，查询等
class CLogicBase
{
public:
	CLogicBase(void) {}
	~CLogicBase(void) {}
public:

	///向数组添加一个数据
	///@param  arr[] 数组, data 要添加的数据, ArrLen 数组的长度
	///@return true 添加成功，false 数组满了，添加不成功
	static bool AddOneDataToArr(BYTE arr[], BYTE data, BYTE ArrLen)
	{
		for (int i = 0;i < ArrLen;i++)
		{
			if (arr[i] == 255)
			{
				arr[i] = data;
				return true;
			}
		}
		return false;
	}

	///从数组添删除一个数据
	///@param  arr[] 数组, data 要删除的数据, ArrLen 数组的长度
	///@return true 添加成功，false 数组满了，添加不成功
	static bool DeleteOneDataFromArr(BYTE arr[], BYTE data, BYTE ArrLen)
	{
		for (int i = 0;i < ArrLen;i++)
		{
			if (arr[i] == data)
			{
				arr[i] = 255;
				return true;
			}
		}
		return false;
	}

	///获取掷色子的随机数据
	///@param  data0 色子1, data1 色子2, data2 色子3
	static void GetSeziData(BYTE *data0 = NULL, BYTE *data1 = NULL, BYTE *data2 = NULL)
	{
		srand((unsigned)time(NULL));
		if (data0 != NULL)
		{
			*data0 = rand() % 6 + 1;
		}
		if (data1 != NULL)
		{
			*data1 = rand() % 6 + 1;
		}
		if (data2 != NULL)
		{
			*data2 = rand() % 6 + 1;
		}
	}

	///获取数组的有效元素个数
	///@param  arr[] 目标数组,data 无效值, ArrLen 数组长度
	///@return 有效元素个数
	static int GetArrInvalidNum(BYTE arr[], BYTE data, BYTE ArrLen)
	{
		int num = 0;
		for (int i = 0;i < ArrLen;i++)
		{
			if (arr[i] != data)
			{
				num++;
			}
		}
		return num;
	}

	///判断数组中是否存在某个元素
	///@param  arr[] 目标数组,data 无效值, ArrLen 数组长度
	///@return 有效元素个数
	static bool IsHaveInArr(BYTE arr[], BYTE data, BYTE ArrLen)
	{
		for (int i = 0;i < ArrLen;i++)
		{
			if (arr[i] == data)
			{
				return true;
			}
		}
		return false;
	}
	///判断数组中是否存在某个元素
	///@param  arr[] 目标数组,data 要查找的元素, ArrLen 数组长度
	///@return 有效元素个数
	static int GetNumInArr(BYTE arr[], BYTE data, BYTE ArrLen)
	{
		int num = 0;
		for (int i = 0;i < ArrLen;i++)
		{
			if (arr[i] == data)
			{
				num++;
			}
		}
		return num;
	}
	///名称：GetPaiNum
	///描述：获取牌数组的有效牌张数
	///@param  pai[] 要检测的牌数组, count 有效牌的张数
	///@return 
	static int GetPaiNum(BYTE pai[], BYTE count)
	{
		int num = 0;
		for (int i = 0;i < count;i++)
		{
			if (pai[i] != 255)
			{
				num++;
			}
		}
		return num;
	}
	///冒泡排序
	//big参数，true：从打到小排序，false：从小到大排序
	static void MaoPaoSort(BYTE a[], int count, bool big)
	{
		if (a == NULL || count <= 1)
			return;
		BYTE byTemp;
		for (int i = 0; i < count - 1; i++)
			for (int j = 0; j < count - i - 1; j++)
			{
				if (big)
				{
					if (a[j] < a[j + 1])
					{
						byTemp = a[j + 1];
						a[j + 1] = a[j];
						a[j] = byTemp;

						/*a[j + 1] = a[j] ^ a[j + 1];
						a[j] = a[j] ^ a[j + 1];
						a[j + 1] = a[j] ^ a[j + 1];*/
					}
				}
				else
				{
					if (a[j] > a[j + 1])
					{
						byTemp = a[j + 1];
						a[j + 1] = a[j];
						a[j] = byTemp;

						/*a[j + 1] = a[j] ^ a[j + 1];
						a[j] = a[j] ^ a[j + 1];
						a[j + 1] = a[j] ^ a[j + 1];*/
					}
				}

			}
	}

	///选择排序
	//big参数，true：从打到小排序，false：从小到大排序
	static void SelectSort(BYTE a[], int count, bool big)
	{
		if (a == NULL || count <= 1)
			return;
		int iMin = 0, iMax = 0;
		BYTE byTemp;
		if (big)
		{
			for (int i = 0; i < count - 1; i++)
			{
				iMax = i;
				for (int j = i + 1; j < count; j++)
					if (a[j] > a[iMax])
						iMax = j;
				if (iMax != i)
				{
					byTemp = a[iMax];
					a[iMax] = a[i];
					a[i] = byTemp;

					/*a[i] = a[i] ^ a[iMax];
					a[iMax] = a[i] ^ a[iMax];
					a[i] = a[i] ^ a[iMax];*/
				}
			}
		}
		else
		{
			for (int i = 0; i < count - 1; i++)
			{
				iMin = i;
				for (int j = i + 1; j < count; j++)
					if (a[j] < a[iMin])
						iMin = j;
				if (iMin != i)
				{
					byTemp = a[iMin];
					a[iMin] = a[i];
					a[i] = byTemp;

					/*a[i] = a[i] ^ a[iMin];
					a[iMin] = a[i] ^ a[iMin];
					a[i] = a[i] ^ a[iMin];*/
				}
			}
		}
	}

	//堆排序
	static void HeapAdjustSmall(BYTE * a, int i, int length) //建立大顶堆，从小到大排序
	{
		int nChild;
		int nTemp;
		for (; 2 * i + 1 < length; i = nChild)
		{
			//子结点的位置=2*（父结点位置）+1
			nChild = 2 * i + 1;
			//得到子结点中较大的结点
			if (nChild<length - 1 && a[nChild + 1]>a[nChild])
				++nChild;
			//如果较大的子结点大于父结点那么把较大的子结点往上移动，替换它的父结点
			if (a[i] < a[nChild])
			{
				nTemp = a[i];
				a[i] = a[nChild];
				a[nChild] = nTemp;
			}
			else break;
		}
	}
	static void HeapSortSmall(BYTE * a, int length)
	{
		int i;
		//调整序列的前半部分元素，调整完之后第一个元素是序列的最大的元素（先建立大顶堆）
		//length/2-1是最后一个非叶节点，此处"/"为整除
		for (i = length / 2 - 1; i >= 0; --i)
			HeapAdjustSmall(a, i, length);

		//从最后一个元素开始对序列进行调整，不断的缩小调整的范围直到第一个元素
		for (i = length - 1; i > 0; --i)
		{
			//把第一个元素和当前的最后一个元素交换，
			//保证当前的最后一个位置的元素都是在现在的这个序列之中最大的
			a[i] = a[0] ^ a[i];
			a[0] = a[0] ^ a[i];
			a[i] = a[0] ^ a[i];
			//不断缩小调整heap的范围，每一次调整完毕保证第一个元素是当前序列的最大值
			HeapAdjustSmall(a, 0, i);
		}
	}

	static void HeapAdjustBig(BYTE * a, int i, int length) //建立小顶堆，从大到小排序
	{
		int nChild;
		int nTemp;
		for (; 2 * i + 1 < length; i = nChild)
		{
			//子结点的位置=2*（父结点位置）+1
			nChild = 2 * i + 1;
			//得到子结点中较大的结点
			if (nChild < length - 1 && a[nChild + 1] < a[nChild])
				++nChild;
			//如果较大的子结点大于父结点那么把较大的子结点往上移动，替换它的父结点
			if (a[i] > a[nChild])
			{
				nTemp = a[i];
				a[i] = a[nChild];
				a[nChild] = nTemp;
			}
			else break;
		}
	}
	static void HeapSortBig(BYTE * a, int length)
	{
		int i;
		//调整序列的前半部分元素，调整完之后第一个元素是序列的最大的元素（先建立大顶堆）
		//length/2-1是最后一个非叶节点，此处"/"为整除
		for (i = length / 2 - 1; i >= 0; --i)
			HeapAdjustBig(a, i, length);

		//从最后一个元素开始对序列进行调整，不断的缩小调整的范围直到第一个元素
		for (i = length - 1; i > 0; --i)
		{
			//把第一个元素和当前的最后一个元素交换，
			//保证当前的最后一个位置的元素都是在现在的这个序列之中最大的
			a[i] = a[0] ^ a[i];
			a[0] = a[0] ^ a[i];
			a[i] = a[0] ^ a[i];
			//不断缩小调整heap的范围，每一次调整完毕保证第一个元素是当前序列的最大值
			HeapAdjustBig(a, 0, i);
		}
	}

	static void HeapSort(BYTE a[], int length, bool big)
	{
		if (length <= 1)
		{
			return;
		}
		if (big)
		{
			HeapSortBig(a, length);
		}
		else
		{
			HeapSortSmall(a, length);
		}
	}

	/**
	* @brief 获取运行程序所在路径名，字符串末尾不加反斜杠
	*/
	static char * G_GetAppPath()
	{
		static char s_Path[MAX_PATH];
		static bool s_bIsReady = false;
		if (!s_bIsReady)
		{
			ZeroMemory(s_Path, sizeof(s_Path));
			DWORD dwLength = GetModuleFileNameA(GetModuleHandle(NULL), s_Path, sizeof(s_Path));
			char *p = /*_tcsrchr*/strrchr(s_Path, '\\');
			*p = '\0';
			s_bIsReady = true;
		}
		return s_Path;
	}
};
#endif
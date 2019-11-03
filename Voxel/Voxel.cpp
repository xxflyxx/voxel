// Voxel.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <vector>
#include <assert.h>

//using namespace vpx;

struct Vector3
{
	float x, y, z;
};

struct Rotation
{
	float pitch;
	float yaw;
	float roll;
};

struct Location : Vector3
{

};

class Transform
{
public:
	Location loc;
	Rotation rot;
};

// 方向编号
enum Direction : uint8_t
{
	Front = 0,
	RF = 1,
	Right = 2,
	RB = 3,
	Back = 4,
	LB = 5,
	Left = 6,
	LF = 7,
};

// 8方向掩码
enum DirectionMask : uint16_t
{
	DirectionMaskFront = 0x0003,
	DirectionMaskRF = 0x000C,
	DirectionMaskRight = 0x0030,
	DirectionMaskRB = 0x00C0,
	DirectionMaskBack = 0x0300,
	DirectionMaskLB = 0x0C00,
	DirectionMaskLeft = 0x3000,
	DirectionMaskLF = 0xC000,
};

// layer邻居关系
enum LayerRelation : uint8_t
{
	Same = 0x00,
	Above = 0x01,
	Low = 0x02,
	Unknow = 0x03
};

class TerrainData
{
public:
	// 每个Grid上的体素链表信息
	struct Voxels
	{
		uint16_t spanIndex;
		uint16_t neighborLayerIndex;
		uint8_t count;
	};

private:
	// 该地图的长宽高
	uint32_t m_length;
	uint32_t m_width;
	uint32_t m_height;
	float m_spanMeasure;
	float m_gridSize;

	// 体素的索引表, 根据x, y定位
	std::vector<Voxels> m_gridArr;

	// 记录体素高度的数组, 用Voxels的spanIndex和count索引, 体素高度=VoxelSpan*m_spanMeasure;
	typedef uint16_t VoxelSpan;
	std::vector<VoxelSpan> m_spanArr;

	// 记录每个体素的8方向邻近关系, 用Voxels的neighborLayerIndex和Count索引
	// 每两个bit表示一个方向, 使用DirctionMask筛选LayerRelation, 00表示位移相同layer, 01表示layer+1, 10表示layer-1, 11表示未知需遍历判断
	typedef uint16_t NeighborLayer;
	std::vector<NeighborLayer> m_neighborLayerArr;

public:
	TerrainData(uint32_t length, uint32_t width, uint32_t height, float spanMeasure = 1.f, float gridSize = 50.f)
		: m_length(length), m_width(width), m_height(height), m_spanMeasure(spanMeasure), m_gridSize(gridSize)
	{
		m_gridArr.resize(m_length*m_width);
	}

	void Import(std::istream& is)
	{
		is >> m_length >> m_width >> m_height;
		m_gridArr.resize(m_length*m_width);
		uint32_t dataCount = 0;
		is >> dataCount;
		for (uint32_t i = 0; i < dataCount; i++)
		{
			int x, y;
			uint8_t layerNum;
			is >> x >> y >> layerNum;
			auto spanCount = layerNum * 2 - 1;
			uint16_t* spans = new uint16_t[spanCount];
			is.read((char*)spans, spanCount * sizeof(uint16_t));
			AddVoxels(x, y, layerNum, spans);
			delete[] spans;
		}
		BuildNeighbor();
	}

	void Export(std::ostream& os)
	{
		os << m_length << m_width << m_height;
		uint32_t dataCount = (uint32_t)m_gridArr.size();
		os << dataCount;
		for (uint32_t j = 0; j < Width(); ++j)
			for (uint32_t i = 0; i < Length(); ++i)
			{
				const auto& vols = GetVoxels(i, j);
				os << i << j << vols.count;
				uint16_t* spans = &m_spanArr[vols.spanIndex];
				os.write((char*)spans, vols.count * sizeof(uint16_t));
			}
	}

	void AddVoxels(uint32_t x, uint32_t y, uint8_t layerNum, uint16_t* spans)
	{
		auto& vols = GetVoxels(x, y);
		m_spanArr.reserve(m_spanArr.size() + layerNum*2);
		vols.spanIndex = (uint16_t)m_spanArr.size();
		vols.count = layerNum;
		for (uint8_t i = 0; i < layerNum*2-1; ++i)
		{
			m_spanArr.push_back(spans[i]);
		}
	}

private:
	// 构建附近关系
	void CalcNeighborRelation(uint32_t x, uint32_t y, Direction dir, uint8_t layer, float hight, uint16_t arrIndex)
	{
		uint8_t dstLayer = 255;
		switch (dir)
		{
		case Front:
			if (x < Length() - 1)
				dstLayer = GetLayer(GetVoxels(x + 1, y), hight);
			break;
		case RF:
			if (x < Length() - 1 && y < Width() - 1)
				dstLayer = GetLayer(GetVoxels(x + 1, y + 1), hight);
			break;
		case Right:
			if (y < Width() - 1)
				dstLayer = GetLayer(GetVoxels(x, y + 1), hight);
			break;
		case RB:
			if (y < Width() - 1 && x > 0)
				dstLayer = GetLayer(GetVoxels(x - 1, y + 1), hight);
			break;
		case Back:
			if (x > 0)
				dstLayer = GetLayer(GetVoxels(x - 1, y), hight);
			break;
		case LB:
			if (x > 0 && y > 0)
				dstLayer = GetLayer(GetVoxels(x - 1, y - 1), hight);
			break;
		case Left:
			if (y > 0)
				dstLayer = GetLayer(GetVoxels(x, y - 1), hight);
			break;
		case LF:
			if (y > 0 && x < Length() - 1)
				dstLayer = GetLayer(GetVoxels(x + 1, y - 1), hight);
			break;
		default:
			break;
		}
		auto rel = dstLayer == layer ? LayerRelation::Same
			: dstLayer == layer + 1 ? LayerRelation::Above 
			: dstLayer == layer - 1 ? LayerRelation::Low : LayerRelation::Unknow;
		m_neighborLayerArr[arrIndex + layer] = uint32_t(rel) << (dir*2);
	}

public:
	// 构建附近关系
	void BuildNeighbor()
	{
		for (uint32_t j = 0; j < Width(); ++j)
			for (uint32_t i = 0; i < Length(); ++i)
			{
				auto& vols = GetVoxels(i, j);
				vols.neighborLayerIndex = (uint16_t)m_neighborLayerArr.size();
				m_neighborLayerArr.resize(m_neighborLayerArr.size() + vols.count);
				for (uint8_t layer = 0; layer < vols.count; ++layer)
				{
					auto hight = GetHight(vols, layer);
					for (auto dir = int(Direction::Front); dir <= int(Direction::LF); ++dir)
						CalcNeighborRelation(i, j, Direction(dir), layer, hight, vols.neighborLayerIndex);
				}
			}
	}

	// 地形总长宽高
	uint32_t Length() const { return m_length; }
	uint32_t Width() const { return m_width; }
	uint32_t Height() const { return m_height; }
	float SpanMeasure() const { return m_spanMeasure; }
	float GridSize() const { return m_gridSize; }

	// 获取坐标对应的体素列表
	const Voxels& GetVoxels(uint32_t x, uint32_t y) const { assert(x < m_length && y < m_width); return m_gridArr[y*m_width + x]; }
	const Voxels& GetVoxels(float x, float y) const { return GetVoxels(uint32_t(x / m_gridSize), uint32_t(y / m_gridSize)); }
	Voxels& GetVoxels(uint32_t x, uint32_t y) { assert(x < m_length && y < m_width); return m_gridArr[y*m_width + x]; }

	// spanIndex 来自 Voxels结构体, layer为grid第几个体素, 注意layer必须<Voxels.count
	float GetVoxelUpper(uint16_t spanIndex, uint8_t layer) const { return m_spanArr[spanIndex + layer * 2 + 1] * m_spanMeasure; }
	float GetVoxelDown(uint16_t spanIndex, uint8_t layer) const { return m_spanArr[spanIndex + layer * 2] * m_spanMeasure; }

	// 获取临近对象的layer
	LayerRelation GetNeighborLayerRelation(const Voxels& vols, uint8_t layer, Direction dir)
	{
		uint16_t offset = dir * 2;
		auto relation = m_neighborLayerArr[vols.neighborLayerIndex + layer] & (0x03 << offset);
		
		return LayerRelation(relation >> offset);
	}

	// 高度计算layer
	uint8_t GetLayer(const Voxels& vols, float hight) const
	{
		uint8_t layer = 0;
		for (auto i = 0; i < vols.count; ++i)
		{
			if (GetVoxelUpper(vols.spanIndex, i) < hight)
				layer = i;
			else
				break;
		}
		return layer;
	}

	// layer计算高
	float GetHight(const Voxels& vols, uint8_t layer) const { return GetVoxelUpper(vols.spanIndex, layer); }
};


class TerrainInstance
{
public:
	struct Grid
	{
		uint16_t maskIndex;
	};

private:
	TerrainData* m_terr;
	// 动态掩码表, 用maskIndex索引
	std::vector<uint8_t> m_maskArr;

	// 体素实例的索引表, x y 定位
	std::vector<Grid> m_gridArr;

public:
	TerrainInstance(TerrainData* terr) : m_terr(terr) {}

	const TerrainData& GetData() const { assert(m_terr); return *m_terr; }
	const Grid& GetGrid(uint32_t x, uint32_t y) { return m_gridArr[y*GetData().Width()+x]; }
	const Grid& GetGrid(float x, float y) { return GetGrid(x/m_terr->GridSize(), y/m_terr->GridSize()); }

	// 掩码
	bool IsMask(const Grid& grid, uint8_t layer)
	{
		return m_maskArr[grid.maskIndex + layer]>0;
	}

	bool IsMask(uint32_t x, uint32_t y, uint8_t layer, uint8_t radius = 0)
	{
		if (radius < 1)
			return IsMask(GetGrid(x, y), layer);
		const auto& t = GetData();
		auto ystart = y > radius ? y - radius : 0;
		auto yend = t.Width() - radius > y ? y + radius : t.Width() - 1;
		auto xstart = x > radius ? x - radius : 0;
		auto xend = t.Length() - radius > x ? x + radius : t.Length() - 1;
		for (auto j = ystart; j <= yend; ++j)
			for (auto i = xstart; i < xend; ++i)
			{
				float hight = t.GetHight(t.GetVoxels(x, y), layer);
				if (IsMask(GetGrid(i, j), t.GetLayer(t.GetVoxels(i, j), hight)))
					return true;
			}
		return false;
	}

	void AddMask(const Grid& grid, uint8_t layer)
	{
		auto& mask = m_maskArr[grid.maskIndex + layer];
		mask += 1;
	}

	void AddMask(uint32_t x, uint32_t y, uint8_t layer, uint8_t radius = 0)
	{
		if (radius < 1)
			return AddMask(GetGrid(x, y), layer);
	}

	void DecMask(const Grid& grid, uint8_t layer)
	{
		auto& mask = m_maskArr[grid.maskIndex + layer];
		mask -= 1;
	}

	void DecMask(uint32_t x, uint32_t y, uint8_t layer, uint8_t radius = 0)
	{
		if (radius < 1)
			return DecMask(GetGrid(x, y), layer);
	}
};


// 封装体素API
class VoxelProxy
{
	TerrainInstance* m_terr;
	TerrainInstance::Grid m_grid;
	TerrainData::Voxels m_vols;

	uint8_t m_layer;
	uint8_t m_radius;

	Location m_loc;
public:
	VoxelProxy(TerrainInstance* terr, const Location& loc, uint8_t radius=0)
		: m_terr(terr), m_radius(radius) { Update(loc);	}

	// 取当前体素上下
	float GetUpper() const { return m_terr->GetData().GetVoxelUpper(m_vols.spanIndex, m_layer); }
	float GetDown() const { return m_terr->GetData().GetVoxelDown(m_vols.spanIndex, m_layer); }

	// 掩码
	bool IsMask() const { return m_terr->IsMask(m_grid, m_layer); }
	void AddMask() { m_terr->AddMask(m_grid, m_layer); }
	void DecMask() { m_terr->DecMask(m_grid, m_layer); }
	
	void Update(const Location& loc)
	{
		m_loc = loc;
		m_vols = m_terr->GetData().GetVoxels(loc.x, loc.y);
		m_grid = m_terr->GetGrid(loc.x, loc.y);
		m_layer = m_terr->GetData().GetLayer(m_vols, loc.z);
	}


};





















int main()
{
    std::cout << "Hello World!\n"; 
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file

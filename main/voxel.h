#pragma once

#include <iostream>
#include <vector>
#include <assert.h>
#include "typedef.h"
#include <functional>

//namespace vpx


// 方向编号
enum class Direction : uint8_t
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
enum class DirectionMask : uint16_t
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
enum class LayerRelation : uint8_t
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

	void StreamRead(std::istream& is, uint32_t& v) { is.read((char*)&v, sizeof(uint32_t)); }
	void StreamWrite(std::ostream& os, uint32_t v) { os.write((char*)&v, sizeof(uint32_t)); }
	void StreamRead(std::istream& is, uint8_t& v) { is.read((char*)&v, sizeof(uint8_t)); }
	void StreamWrite(std::ostream& os, uint8_t v) { os.write((char*)&v, sizeof(uint8_t)); }
public:
	TerrainData(uint32_t length, uint32_t width, uint32_t height, float spanMeasure = 1.f, float gridSize = 50.f)
		: m_length(length), m_width(width), m_height(height), m_spanMeasure(spanMeasure), m_gridSize(gridSize)
	{
		m_gridArr.resize(m_length*m_width);
	}

	// 导入
	void Import(std::istream& is)
	{
		StreamRead(is, m_length);
		StreamRead(is, m_width);
		StreamRead(is, m_height);
		m_gridArr.resize(m_length*m_width);
		uint32_t dataCount = 0;
		StreamRead(is, dataCount);
		for (uint32_t i = 0; i < dataCount; i++)
		{
			uint32_t x, y;
			uint8_t layerNum;
			StreamRead(is, x);
			StreamRead(is, y);
			StreamRead(is, layerNum);
			auto spanCount = SpanCount(layerNum);
			uint16_t* spans = new uint16_t[spanCount];
			is.read((char*)spans, spanCount * sizeof(uint16_t));
			AddVoxels(x, y, layerNum, spans);
			delete[] spans;
		}
		BuildNeighbor();
	}

	// 导出
	void Export(std::ostream& os)
	{
		StreamWrite(os, m_length);
		StreamWrite(os, m_width);
		StreamWrite(os, m_height);
		uint32_t dataCount = (uint32_t)m_gridArr.size();
		StreamWrite(os, dataCount);
		for (uint32_t j = 0; j < Width(); ++j)
			for (uint32_t i = 0; i < Length(); ++i)
			{
				const auto& vols = GetVoxels(i, j);
				StreamWrite(os, i);
				StreamWrite(os, j);
				StreamWrite(os, vols.count);
				uint16_t* spans = &m_spanArr[vols.spanIndex];
				os.write((char*)spans, SpanCount(vols.count) * sizeof(uint16_t));
			}
	}

	// 添加一列体素, 高度为换算值
	void AddVoxels(uint32_t x, uint32_t y, uint8_t layerNum, uint16_t* spans)
	{
		auto& vols = GetVoxels(x, y);
		vols.spanIndex = (uint16_t)m_spanArr.size();
		vols.count = layerNum;
		m_spanArr.reserve(m_spanArr.size() + SpanCount(layerNum));
		for (uint8_t i = 0; i < SpanCount(layerNum); ++i)
		{
			m_spanArr.push_back(spans[i]);
		}
	}

	// 添加一列体素, 高度为浮点值
	void AddVoxels(uint32_t x, uint32_t y, uint8_t layerNum, float* spans)
	{
		auto sz = new uint16_t[SpanCount(layerNum)];
		for (uint8_t i = 0; i < SpanCount(layerNum); ++i)
		{
			sz[i] = uint16_t(spans[i]/SpanMeasure());
		}
		AddVoxels(x, y, layerNum, sz);
		delete[] sz;
	}
private:
	// 构建附近关系
	void CalcNeighborRelation(uint32_t x, uint32_t y, Direction dir, uint8_t layer, float hight, uint16_t arrIndex)
	{
		uint8_t dstLayer = 255;
		switch (dir)
		{
		case Direction::Front:
			if (x < Length() - 1)
				dstLayer = GetLayer(GetVoxels(x + 1, y), hight);
			break;
		case Direction::RF:
			if (x < Length() - 1 && y < Width() - 1)
				dstLayer = GetLayer(GetVoxels(x + 1, y + 1), hight);
			break;
		case Direction::Right:
			if (y < Width() - 1)
				dstLayer = GetLayer(GetVoxels(x, y + 1), hight);
			break;
		case Direction::RB:
			if (y < Width() - 1 && x > 0)
				dstLayer = GetLayer(GetVoxels(x - 1, y + 1), hight);
			break;
		case Direction::Back:
			if (x > 0)
				dstLayer = GetLayer(GetVoxels(x - 1, y), hight);
			break;
		case Direction::LB:
			if (x > 0 && y > 0)
				dstLayer = GetLayer(GetVoxels(x - 1, y - 1), hight);
			break;
		case Direction::Left:
			if (y > 0)
				dstLayer = GetLayer(GetVoxels(x, y - 1), hight);
			break;
		case Direction::LF:
			if (y > 0 && x < Length() - 1)
				dstLayer = GetLayer(GetVoxels(x + 1, y - 1), hight);
			break;
		default:
			break;
		}
		auto rel = dstLayer == layer ? LayerRelation::Same
			: dstLayer == layer + 1 ? LayerRelation::Above 
			: dstLayer == layer - 1 ? LayerRelation::Low : LayerRelation::Unknow;
		m_neighborLayerArr[arrIndex + layer] |= uint32_t(rel) << (uint8_t(dir)*2);
	}

public:
	// 根据原layer 和 LayerRelation 得到 目标layer;
	static uint8_t RelationToLayer(uint8_t layer, LayerRelation rel)
	{
		check(rel != LayerRelation::Unknow);
		return rel == LayerRelation::Same ? layer : rel == LayerRelation::Above ? layer + 1 : rel == LayerRelation::Low ? layer - 1 : 0;
	}

	// 体素数量转span数量
	static uint8_t SpanCount(uint8_t layerNum) { return layerNum * 2 - 1; }

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
					for (auto dir = uint8_t(Direction::Front); dir <= uint8_t(Direction::LF); ++dir)
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
	//const Voxels& GetVoxels(float x, float y) const { return GetVoxels(uint32_t(x / m_gridSize), uint32_t(y / m_gridSize)); }
	Voxels& GetVoxels(uint32_t x, uint32_t y) { assert(x < m_length && y < m_width); return m_gridArr[y*m_width + x]; }

	// spanIndex 来自 Voxels结构体, layer为grid第几个体素, 注意layer必须<Voxels.count
	float GetVoxelUpper(uint16_t spanIndex, uint8_t layer) const { return m_spanArr[spanIndex + layer * 2] * m_spanMeasure; }
	float GetVoxelDown(uint16_t spanIndex, uint8_t layer) const { return layer == 0 ? 0.f : m_spanArr[spanIndex + layer * 2 - 1] * m_spanMeasure; }

	// 获取临近对象的layer
	LayerRelation GetNeighborLayerRelation(const Voxels& vols, uint8_t layer, Direction dir) const
	{
		uint16_t offset = uint8_t(dir) * 2;
		auto relation = m_neighborLayerArr[vols.neighborLayerIndex + layer] & (0x03 << offset);
		
		return LayerRelation(relation >> offset);
	}

	// 遍历layer寻找根据高度合适的layer
	uint8_t GetLayer(const Voxels& vols, float hight) const
	{
		uint8_t layer = 0;
		for (uint8_t i = 0; i < vols.count; ++i)
		{
			float v = GetVoxelUpper(vols.spanIndex, i);
			if (v <= hight)
				layer = i;
			else
				break;
		}
		return layer;
	}

	// 遍历寻找误差范围内的layer
	uint8_t GetLayer(const Voxels& vols, float hight, float up, float down) const
	{
		for (uint8_t i = 0; i < vols.count; ++i)
		{
			float v = GetVoxelUpper(vols.spanIndex, i);
			if (v + up >= hight && v - down <= hight)
				return i;
		}
	}

	// layer计算高
	float GetHight(const Voxels& vols, uint8_t layer) const { return GetVoxelUpper(vols.spanIndex, layer); }

	void CalcDirectionGrid(Direction dir, uint32_t& x, uint32_t& y) const
	{
	}
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
	TerrainInstance(TerrainData* terr) : m_terr(terr)
	{
		m_gridArr.resize(terr->Length()*terr->Width());
		for(uint32_t x=0;x<terr->Length();++x)
			for (uint32_t y=0;y<terr->Width();++y)
			{
				const auto& vx = terr->GetVoxels(x, y);
				m_gridArr[y*GetData().Width() + x].maskIndex = (uint8_t)m_maskArr.size();
				m_maskArr.resize(m_maskArr.size() + vx.count, 0);
			}
	}

	const TerrainData& GetData() const { assert(m_terr); return *m_terr; }
	const Grid& GetGrid(uint32_t x, uint32_t y) const { return m_gridArr[y*GetData().Width()+x]; }
	//const Grid& GetGrid(float x, float y) { return GetGrid(x/m_terr->GridSize(), y/m_terr->GridSize()); }

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

	// todo: 半径没实现
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

	// todo: 半径没实现
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
	uint32_t m_gridX;
	uint32_t m_gridY;
public:
	VoxelProxy(TerrainInstance* terr, const Location& loc, uint8_t radius=0)
		: m_terr(terr), m_radius(radius) { Update(loc);	}

	const Location& GetLocation() const { return m_loc; }

	// 取当前体素上下
	float GetUpper() const { return m_terr->GetData().GetVoxelUpper(m_vols.spanIndex, m_layer); }
	float GetDown() const { return m_terr->GetData().GetVoxelDown(m_vols.spanIndex, m_layer); }

	// 掩码
	bool IsMask() const { return m_terr->IsMask(m_grid, m_layer); }
	void AddMask() { m_terr->AddMask(m_grid, m_layer); }
	void DecMask() { m_terr->DecMask(m_grid, m_layer); }
	
	// 更新位置
	void Update(const Location& loc)
	{
		m_loc = loc;
		m_gridX = uint32_t(loc.x / m_terr->GetData().GridSize());
		m_gridY = uint32_t(loc.y / m_terr->GetData().GridSize());

		m_vols = m_terr->GetData().GetVoxels(m_gridX, m_gridY);
		m_grid = m_terr->GetGrid(m_gridX, m_gridY);
		m_layer = GetLayer(m_vols, loc.z);
	}

	// 位移失败返回false, bug: 中间可能跨越了多个格子
	bool MoveTo(const Location& loc)
	{
		auto gridX = uint32_t(loc.x / m_terr->GetData().GridSize());
		auto gridY = uint32_t(loc.y / m_terr->GetData().GridSize());
		const auto& vols = m_terr->GetData().GetVoxels(gridX, gridY);
		auto rel = GetRelation(gridX, gridY);
		if (rel == LayerRelation::Unknow)
		{
			return false;
		}
		m_loc = loc;
		m_gridX = gridX;
		m_gridY = gridY;
		m_vols = vols;
		m_grid = m_terr->GetGrid(gridX, gridY);
		m_layer = m_terr->GetData().RelationToLayer(m_layer, rel);
		m_loc.z = GetUpper();
		return true;
	}

	// x y 计算体素
	const TerrainData::Voxels& GetVoxels(uint32_t x, uint32_t y) const
	{
		return m_terr->GetData().GetVoxels(x, y);
	}

	//  计算 dir 对应 X Y
	void CalcDirectionGrid(Direction dir, uint32_t& x, uint32_t& y) const
	{
		m_terr->GetData().CalcDirectionGrid(dir, x, y);
	}

	// 获取体素的 layer
	uint8_t GetLayer(const TerrainData::Voxels& vol, float hight) const
	{
		return m_terr->GetData().GetLayer(vol, hight);
	}

	// 取得 对应的layer关系
	LayerRelation GetRelation(uint32_t x, uint32_t y) const
	{
		LayerRelation rel = LayerRelation::Unknow;

		int offX = int(x - m_gridX);
		int offY = int(y - m_gridY);

		if (offX > 1 || offX < -1 || offY > 1 || offY < -1)
			return rel;
		else
		{
			Direction dir = Direction::Front;
			switch (offY)
			{
			case -1:
				switch (offX)
				{
				case -1:
					dir = Direction::LB; break;
				case 0:
					dir = Direction::Left; break;
				case 1:
					dir = Direction::LF; break;
				}
				break;
			case 0:
				switch (offX)
				{
				case -1:
					dir = Direction::Back; break;
				case 0:
					return LayerRelation::Same; break;
				case 1:
					dir = Direction::Front; break;
				}
				break;
			case 1:
				switch (offX)
				{
				case -1:
					dir = Direction::RB; break;
				case 0:
					dir = Direction::Right; break;
				case 1:
					dir = Direction::RF; break;
				}
				break;
			}
			return m_terr->GetData().GetNeighborLayerRelation(m_vols, m_layer, dir);
		}
	}

	// 获取位置对应的关系
	LayerRelation GetRelation(const Location& loc) const
	{
		auto gridX = uint32_t(loc.x / m_terr->GetData().GridSize());
		auto gridY = uint32_t(loc.y / m_terr->GetData().GridSize());

		return GetRelation(gridX, gridY);
	}


	// 遍历 周围体素
	void GetNeighborGrid(std::function<void(uint32_t, uint32_t, uint8_t/*layer*/)> cb) const
	{
		for (uint8_t i = 0; i <= uint8_t(Direction::LF); ++i)
		{
			m_terr->GetData().GetNeighborLayerRelation(m_vols, m_layer, Direction(i));
			uint32_t x, y;
			CalcDirectionGrid(Direction(i), x, y);
			const auto& vols = GetVoxels(x, y);
			cb(x, y, GetLayer(vols, m_loc.z));
		}
	}
};






















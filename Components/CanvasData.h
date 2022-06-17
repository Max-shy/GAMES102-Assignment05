#pragma once

#include <UGM/UGM.h>

struct CanvasData {
	std::vector<Ubpa::pointf2> points;//点序列
	Ubpa::valf2 scrolling{ 0.f,0.f };
	bool opt_enable_grid{ true };
	bool opt_enable_context_menu{ true };
	bool adding_line{ false };
	bool opt_InitLine{ true };
	bool opt_Initpoints{ true };

	//细分曲线
	std::vector<Ubpa::pointf2> Chaikin_Points;//二次均匀B样条细分点序列
	std::vector<Ubpa::pointf2> Cubic_Points;//三次均匀B样条细分点序列
	std::vector<Ubpa::pointf2> Interpolation_Points;//四点插值细分点序列

	//细分开关
	bool opt_Chaikin{ true };
	bool opt_Cubic{false };
	bool opt_Interpolation{ false };

	//闭合开关
	bool IsClose{ true };

	////参数化
	//bool opt_Uniform{ false };//均匀参数化
	//bool opt_Chordal{ false };//Chordal参数化
	//bool opt_Centripetal{ false };//平方中心参数化
	//bool opt_Foley{ true };//Foley参数化

	float alpha = 0.125;//四点插值法，参数
	int Subdivision_Level=0;//细分次数
};

#include "details/CanvasData_AutoRefl.inl"

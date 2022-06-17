#pragma once

#include <UGM/UGM.h>

struct CanvasData {
	std::vector<Ubpa::pointf2> points;//������
	Ubpa::valf2 scrolling{ 0.f,0.f };
	bool opt_enable_grid{ true };
	bool opt_enable_context_menu{ true };
	bool adding_line{ false };
	bool opt_InitLine{ true };
	bool opt_Initpoints{ true };

	//ϸ������
	std::vector<Ubpa::pointf2> Chaikin_Points;//���ξ���B����ϸ�ֵ�����
	std::vector<Ubpa::pointf2> Cubic_Points;//���ξ���B����ϸ�ֵ�����
	std::vector<Ubpa::pointf2> Interpolation_Points;//�ĵ��ֵϸ�ֵ�����

	//ϸ�ֿ���
	bool opt_Chaikin{ true };
	bool opt_Cubic{false };
	bool opt_Interpolation{ false };

	//�պϿ���
	bool IsClose{ true };

	////������
	//bool opt_Uniform{ false };//���Ȳ�����
	//bool opt_Chordal{ false };//Chordal������
	//bool opt_Centripetal{ false };//ƽ�����Ĳ�����
	//bool opt_Foley{ true };//Foley������

	float alpha = 0.125;//�ĵ��ֵ��������
	int Subdivision_Level=0;//ϸ�ִ���
};

#include "details/CanvasData_AutoRefl.inl"

#include "CanvasSystem.h"

#include "../Components/CanvasData.h"

#include <_deps/imgui/imgui.h>

using namespace Ubpa;

//细分函数
std::vector<Ubpa::pointf2> Chaikin_Subdivision(std::vector<Ubpa::pointf2> points, int Level, bool IsClose);
std::vector<Ubpa::pointf2> Cubic_Subdivision(std::vector<Ubpa::pointf2> points, int Level, bool IsClose);
std::vector<Ubpa::pointf2> Interpolation_Subdivision(std::vector<Ubpa::pointf2> points, int Level, bool IsClose,float alpha);

//绘制曲线
void DrawCurve(ImDrawList* draw_list, float origin_x, float origin_y, std::vector<pointf2> draw_points, ImU32 color,bool IsClose);


void CanvasSystem::OnUpdate(Ubpa::UECS::Schedule& schedule) {
	schedule.RegisterCommand([](Ubpa::UECS::World* w) {
		auto data = w->entityMngr.GetSingleton<CanvasData>();
		if (!data)
			return;

		if (ImGui::Begin("Canvas")) {
			ImGui::Checkbox("Enable grid", &data->opt_enable_grid);
			ImGui::Checkbox("Enable context menu", &data->opt_enable_context_menu);
			ImGui::Text("Mouse Left: drag to add lines,\nMouse Right: drag to scroll, click for context menu.");

			ImGui::Checkbox("Chaikin Subdivision", &data->opt_Chaikin);
			//ImGui::SameLine(214); ImGui::Checkbox("Uniform", &data->opt_Uniform);
			//ImGui::SameLine(); ImGui::Checkbox("Chordal", &data->opt_Chordal);
			//ImGui::SameLine(); ImGui::Checkbox("Centripetal", &data->opt_Centripetal);
			//ImGui::SameLine(); ImGui::Checkbox("Foley", &data->opt_Foley);
			ImGui::SameLine(214);ImGui::Checkbox("IsClose", &data->IsClose);
			ImGui::Checkbox("Cubic Subdivision", &data->opt_Cubic);
			ImGui::SameLine(214);
			ImGui::InputInt("Subdivision Level", &data->Subdivision_Level);
			ImGui::Checkbox("Interpolation Subdivision", &data->opt_Interpolation);
			ImGui::SameLine();
			ImGui::InputFloat("alpha", &data->alpha);

			// Using InvisibleButton() as a convenience 1) it will advance the layout cursor and 2) allows us to use IsItemHovered()/IsItemActive()
			ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();      // ImDrawList API uses screen coordinates!
			ImVec2 canvas_sz = ImGui::GetContentRegionAvail();   // Resize canvas to what's available
			if (canvas_sz.x < 50.0f) canvas_sz.x = 50.0f;
			if (canvas_sz.y < 50.0f) canvas_sz.y = 50.0f;
			ImVec2 canvas_p1 = ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);

			// Draw border and background color
			ImGuiIO& io = ImGui::GetIO();
			ImDrawList* draw_list = ImGui::GetWindowDrawList();
			draw_list->AddRectFilled(canvas_p0, canvas_p1, IM_COL32(50, 50, 50, 255));
			draw_list->AddRect(canvas_p0, canvas_p1, IM_COL32(255, 255, 255, 255));

			// This will catch our interactions
			ImGui::InvisibleButton("canvas", canvas_sz, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
			const bool is_hovered = ImGui::IsItemHovered(); // Hovered
			const bool is_active = ImGui::IsItemActive();   // Held
			const ImVec2 origin(canvas_p0.x + data->scrolling[0], canvas_p0.y + data->scrolling[1]); // 原点
			const pointf2 mouse_pos_in_canvas(io.MousePos.x - origin.x, io.MousePos.y - origin.y);


			// Add point
			if (is_hovered &&/* !data->adding_line &&*/ ImGui::IsMouseClicked(ImGuiMouseButton_Left))
			{
				data->points.push_back(mouse_pos_in_canvas);
			}

			// Pan (we use a zero mouse threshold when there's no context menu)
			// You may decide to make that threshold dynamic based on whether the mouse is hovering something etc.
			const float mouse_threshold_for_pan = data->opt_enable_context_menu ? -1.0f : 0.0f;
			if (is_active && ImGui::IsMouseDragging(ImGuiMouseButton_Right, mouse_threshold_for_pan))
			{
				data->scrolling[0] += io.MouseDelta.x;
				data->scrolling[1] += io.MouseDelta.y;
			}

			// Context menu (under default mouse threshold)
			ImVec2 drag_delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);
			if (data->opt_enable_context_menu && ImGui::IsMouseReleased(ImGuiMouseButton_Right) && drag_delta.x == 0.0f && drag_delta.y == 0.0f)
				ImGui::OpenPopupContextItem("context");
			if (ImGui::BeginPopup("context"))
			{
				if (data->adding_line)
					data->points.resize(data->points.size() - 2);
				data->adding_line = false;
				if (ImGui::MenuItem("Remove one", NULL, false, data->points.size() > 0)) {
					data->points.resize(data->points.size() - 1);
				}
				if (ImGui::MenuItem("Remove all", NULL, false, data->points.size() > 0)) {
					data->points.clear();
				}
				ImGui::EndPopup();
			}

			// Draw grid + all lines in the canvas
			draw_list->PushClipRect(canvas_p0, canvas_p1, true);
			if (data->opt_enable_grid)
			{
				const float GRID_STEP = 64.0f;
				for (float x = fmodf(data->scrolling[0], GRID_STEP); x < canvas_sz.x; x += GRID_STEP)
					draw_list->AddLine(ImVec2(canvas_p0.x + x, canvas_p0.y), ImVec2(canvas_p0.x + x, canvas_p1.y), IM_COL32(200, 200, 200, 40));
				for (float y = fmodf(data->scrolling[1], GRID_STEP); y < canvas_sz.y; y += GRID_STEP)
					draw_list->AddLine(ImVec2(canvas_p0.x, canvas_p0.y + y), ImVec2(canvas_p1.x, canvas_p0.y + y), IM_COL32(200, 200, 200, 40));
			}
			
			//绘制点列、曲线
			for (int i = 0; i < data->points.size(); i++) {
				draw_list->AddCircleFilled(ImVec2(data->points[i][0] + origin.x, data->points[i][1] + origin.y), 4.0f, IM_COL32(255, 255, 0, 255));
			}
			if (data->points.size() > 1) {
				for (int i = 0; i < data->points.size() - 1; i++) {
					draw_list->AddLine(ImVec2(data->points[i][0] + origin.x, data->points[i][1] + origin.y), ImVec2(data->points[i + 1][0] + origin.x, data->points[i + 1][1] + origin.y), IM_COL32(0, 255, 0, 255), 2.0f);
				}
				if (data->IsClose && data->points.size() > 2) {
					draw_list->AddLine(ImVec2(data->points[data->points.size() - 1][0] + origin.x, data->points[data->points.size() - 1][1] + origin.y), ImVec2(data->points[0][0] + origin.x, data->points[0][1] + origin.y), IM_COL32(0, 255, 0, 255), 2.0f);
				}
			}

			

			//绘制细分曲线
			if (data->points.size() > 3) {
				std::vector<Ubpa::pointf2> draw_points;
				if (data->opt_Chaikin) {
					//计算二次B样条细分、并绘制
					draw_points = Chaikin_Subdivision(data->points, data->Subdivision_Level,data->IsClose);
					DrawCurve(draw_list, origin.x, origin.y, draw_points, IM_COL32(255, 128, 128, 255), data->IsClose);
				}
				if (data->opt_Cubic) {
					//计算三次B样条细分，并绘制
					draw_points = Cubic_Subdivision(data->points, data->Subdivision_Level,data->IsClose);
					DrawCurve(draw_list, origin.x, origin.y, draw_points, IM_COL32(255, 64, 64, 255), data->IsClose);
				}
				if (data->opt_Interpolation) {
					//计算四点插值细分，并绘制
					draw_points = Interpolation_Subdivision(data->points, data->Subdivision_Level,data->IsClose, data->alpha);
					DrawCurve(draw_list, origin.x, origin.y, draw_points, IM_COL32(254, 67, 101, 255), data->IsClose);
				}
		
			}

			//绘制点列
			for (int n = 0; n < data->points.size() ; n++) {
				draw_list->AddCircleFilled(ImVec2(data->points[n][0] + origin.x, data->points[n][1] + origin.y), 4.0f, IM_COL32(255, 255, 0, 255));
			}

			draw_list->PopClipRect();
		
		}
		ImGui::End();
	});
}

//二次B样条均匀细分
std::vector<Ubpa::pointf2> Chaikin_Subdivision(std::vector<Ubpa::pointf2> points, int Level,bool IsClose)
{
	if (Level == 0) {
		return points;
	}
	std::vector<Ubpa::pointf2> Chaikin_points;
	Chaikin_points.clear();
	int n = points.size();
	for (int i = 0; i < points.size(); i++) {
		float x1 = points[(i - 1 + n) % n][0] * 0.25f + points[i][0] * 0.75f;
		float y1 = points[(i - 1 + n) % n][1] * 0.25f + points[i][1] * 0.75f;
		float x2 = points[i][0] * 0.75f + points[(i + 1) % n][0] * 0.25f;
		float y2 = points[i][1] * 0.75f + points[(i + 1) % n][1] * 0.25f;
		Chaikin_points.push_back(pointf2(x1, y1));//v_2i
		if (i == 0&&!IsClose)
			Chaikin_points.pop_back();
		Chaikin_points.push_back(pointf2(x2, y2));//v_2i+1
	}
	if(!IsClose)
		Chaikin_points.pop_back();
	return Chaikin_Subdivision(Chaikin_points, Level - 1, IsClose);
}

//三次B样条均匀细分
std::vector<Ubpa::pointf2> Cubic_Subdivision(std::vector<Ubpa::pointf2> points, int Level, bool IsClose)
{
	if (Level == 0) {
		return points;
	}
	std::vector<Ubpa::pointf2> Cubic_points;
	Cubic_points.clear();
	int n = points.size();
	for (int i = 0; i < points.size(); i++) {
		float x1 = points[(i - 1 + n) % n][0] * 0.125f + points[i][0] * 0.75f+ points[(i + 1) % n][0]*0.125;
		float y1 = points[(i - 1 + n) % n][1] * 0.125f + points[i][1] * 0.75f+ points[(i + 1) % n][1]*0.125;
		float x2 = points[i][0] * 0.5f + points[(i + 1) % n][0] * 0.5f;
		float y2 = points[i][1] * 0.5f + points[(i + 1) % n][1] * 0.5f;
		Cubic_points.push_back(pointf2(x1, y1));//v_2i
		/*if (i == 0 && !IsClose)
			Cubic_points.pop_back();*/
		Cubic_points.push_back(pointf2(x2, y2));//v_2i+1
	}
	if (!IsClose)
		Cubic_points.pop_back();
	return Cubic_Subdivision(Cubic_points, Level - 1, IsClose);
}

std::vector<Ubpa::pointf2> Interpolation_Subdivision(std::vector<Ubpa::pointf2> points, int Level, bool IsClose, float alpha) {
	if (Level == 0)
		return points;
	std::vector<Ubpa::pointf2> Inter_points;
	Inter_points.clear();
	int n = points.size();
	for (int i = 0; i < points.size(); i++) {
		float x = 0.5 * (points[i][0] + points[(i + 1) % n][0]) + alpha * 0.5 * (points[i][0] + points[(i + 1) % n][0] - points[(i - 1 + n) % n][0] - points[(i + 2) % n][0]);
		float y = 0.5 * (points[i][1] + points[(i + 1) % n][1]) + alpha * 0.5 * (points[i][1] + points[(i + 1) % n][1] - points[(i - 1 + n) % n][1] - points[(i + 2) % n][1]);
		Inter_points.push_back(points[i]);//v_2i
		//if (i == 0 && !IsClose)
		//	Inter_points.pop_back();
		Inter_points.push_back(pointf2(x, y));//v_2i+1
	}
	if (!IsClose) {
		Inter_points.pop_back();
	}
	return Interpolation_Subdivision(Inter_points, Level - 1, IsClose, alpha);
}

//曲线绘制
void DrawCurve(ImDrawList* draw_list, float origin_x, float origin_y, std::vector<pointf2> draw_points, ImU32 color,bool IsClose) {
	int n = draw_points.size();
	//绘制点列
	for (int n = 0; n < draw_points.size(); n++) {
		draw_list->AddCircleFilled(ImVec2(draw_points[n][0] + origin_x, draw_points[n][1] + origin_y), 4.0f, IM_COL32(128, 255, 255, 255));
	}
	if (IsClose) {
		for (int i = 0; i < n ; i++) {
			draw_list->AddLine(ImVec2(origin_x + draw_points[(i-1+n)%n][0], origin_y + draw_points[(i - 1 + n) % n][1]), ImVec2(origin_x + draw_points[i][0], origin_y + draw_points[i][1]), color, 2.0f);
		}
	}
	else {
		for (int i = 0; i < n-1; i++) {
			draw_list->AddLine(ImVec2(origin_x + draw_points[i][0], origin_y + draw_points[i][1]), ImVec2(origin_x + draw_points[i+1][0], origin_y + draw_points[i+1][1]), color, 2.0f);
		}
	}
	
}
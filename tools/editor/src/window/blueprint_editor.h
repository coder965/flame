#pragma once

#include <flame/engine/ui/ui.h>

struct BlueprintEditor : flame::ui::Window
{
	struct Node
	{
		int     ID;
		char    Name[32];
		ImVec2  Pos, Size;
		float   Value;
		ImVec4  Color;
		int     InputsCount, OutputsCount;

		Node(int id, const char* name, const ImVec2& pos, float value, const ImVec4& color, int inputs_count, int outputs_count);

		ImVec2 GetInputSlotPos(int slot_no) const;
		ImVec2 GetOutputSlotPos(int slot_no) const;
	};

	struct NodeLink
	{
		int InputIdx, InputSlot, OutputIdx, OutputSlot;

		NodeLink(int input_idx, int input_slot, int output_idx, int output_slot);
	};

	ImVector<Node> nodes;
	ImVector<NodeLink> links;
	ImVec2 scrolling;
	bool show_grid;
	int node_selected;

	BlueprintEditor();
	~BlueprintEditor();
	virtual void on_show() override;
};

extern BlueprintEditor *blueprint_editor;

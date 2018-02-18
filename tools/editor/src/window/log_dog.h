#pragma once

#include <flame/ui/ui.h>

struct LogDog : tke::ui::Window
{
	struct Column
	{
		char name[20];
		int match_index;

		Column();
	};

	std::string log_filename;
	long long log_file_timestamp;

	char match_regex[100];
	std::vector<std::unique_ptr<Column>> columns;

	std::vector<std::vector<std::string>> logs;
	int curr_page;

	LogDog();
	~LogDog();
	void set_log_filename(const std::string &filename);
	virtual void on_show() override;
};

extern LogDog *log_dog;

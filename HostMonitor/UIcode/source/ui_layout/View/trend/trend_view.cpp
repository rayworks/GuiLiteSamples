#include "../include/GuiLite.h"
#include "../include/ctrl_id.h"
#include "../../../manager/value_manager.h"
#include "../source/data/database.h"
#include "../source/ui_ctrl_ex/trend_graph.h"
#include "../source/ui_ctrl_ex/trend_table.h"
#include "../source/ui_ctrl_ex/time_bar.h"
#include "trend_view.h"

#define HR_COLOR				GL_RGB(148,251,78)
#define SPO2_COLOR				GL_RGB(90,166,247)
#define RR_COLOR				GL_RGB(255,255,82)
#define NIBP_COLOR				GL_RGB(255,0,0)

#define H_AXIS_MARK_CNT			7
#define H_AXIS_MARK_INTERVAL	(60 * 10)	//10 minutes between 2 marks

void c_trend_view::on_init_children(void)
{
	c_trend_graph* p_trend_graph = (c_trend_graph*)get_wnd_ptr(ID_TREND_SCREEN_VITAL_TREND_CTRL);
	p_trend_graph->set_type(TREND_TYPE_VITAL);
	p_trend_graph = (c_trend_graph*)get_wnd_ptr(ID_TREND_SCREEN_PRESSURE_CTRL);
	p_trend_graph->set_type(TREND_TYPE_NIBP);
	register_timer((60 * 1000), database_timer_callback, this);
}

void c_trend_view::on_paint(void)
{
	c_rect rect;
	get_screen_rect(rect);
	m_surface->draw_rect(rect.m_left-2, rect.m_top-2, rect.m_right,	rect.m_bottom-5, GL_RGB(99,101,99), m_z_order);
	m_surface->draw_rect(rect.m_left-1, rect.m_top-1, rect.m_right-1,	rect.m_bottom-6, GL_RGB(123,125,123), m_z_order);
	m_surface->draw_rect(rect.m_left,   rect.m_top,   rect.m_right-2,  rect.m_bottom-7, GL_RGB(165,166,156), m_z_order);
	m_surface->draw_rect(rect.m_left+1, rect.m_top+1, rect.m_right-3,	rect.m_bottom-8, GL_RGB(123,125,123), m_z_order);
	m_surface->draw_rect(rect.m_left+2, rect.m_top+2, rect.m_right-4,	rect.m_bottom-9, GL_RGB(99,101,99), m_z_order);

	static bool first_show=true;
	if (first_show)
	{
		database_timer_callback(this);
		first_show = false;
	}
}

void c_trend_view::update_trend_view()
{
	c_time_bar*  p_time_bar = (c_time_bar*)get_wnd_ptr(ID_TREND_SCREEN_TIME_SCALE_BAR_CTRL);
	p_time_bar->set_time(get_time_in_second());
	refresh_trend_table(p_time_bar->get_start_time(), p_time_bar->get_end_time());
	refresh_trend_graphic(p_time_bar->get_end_time());
}

void c_trend_view::refresh_trend_table(long start_time, long end_time)
{
	c_trend_table* p_table = (c_trend_table*)get_wnd_ptr(ID_TREND_SCREEN_TABLE_CTRL);
	if ( p_table == 0 )
	{
		ASSERT(false);
		return;
	}

	//time row
	T_TIME time;
	unsigned int color;
	char str_buf[32];
	for (int i = 1; i <= TREND_TABLE_DATA_CNT; i++)
	{
		color = (i % 2) ? ROW_EVEN_COLOR : ROW_ODD_COLOR;
		if (i == TREND_TABLE_DATA_CNT) { color = LATEST_COL_COLOR; }

		// 1st row
		time = second_to_day((start_time + (i - 1) * 60));
		sprintf(str_buf, "   %02d:%02d ", time.hour, time.minute);
		p_table->set_item(0, i, str_buf, color);
		// 2nd row
		p_table->set_item(1, i, "    ", color);//for vital - tbd
	}
	//data rows
	int data[7][TREND_TABLE_DATA_CNT];//hr, spo2, rr, pr, sys, dia, mean
	read_trend_data(start_time, end_time,
		data[0], data[1], data[2], data[3], data[4], data[5], data[6],
		TREND_TABLE_DATA_CNT);
	for (int i = 0; i < 7; i++)
	{
		for (int j = 1; j <= TREND_TABLE_DATA_CNT; j++)
		{
			color = (j % 2) ? ROW_EVEN_COLOR : ROW_ODD_COLOR;
			if (j == 8) { color = LATEST_COL_COLOR; }

			if (data[i][j - 1] != XXX) 
			{
				sprintf(str_buf, "    %d", data[i][j - 1]); 
			}
			else
			{
				sprintf(str_buf, "      --- ");
			}
			p_table->set_item((i + 2), j, str_buf, color);
		}
	}
}

void c_trend_view::refresh_trend_graphic(long time)
{
	c_trend_graph*	p_vitals_trend_graph = (c_trend_graph*)get_wnd_ptr(ID_TREND_SCREEN_VITAL_TREND_CTRL);
	c_trend_graph*	p_nibp_trend = (c_trend_graph*)get_wnd_ptr(ID_TREND_SCREEN_PRESSURE_CTRL);
	if (!p_vitals_trend_graph || !p_nibp_trend)
	{
		ASSERT(false);
		return;
	}
	
	//update axis
	long x_axis_marks[H_AXIS_MARK_CNT];
	for (long i = 0; i < H_AXIS_MARK_CNT; i++)
	{
		x_axis_marks[(H_AXIS_MARK_CNT - 1) - i] = (time - (i * H_AXIS_MARK_INTERVAL));
	}
	p_vitals_trend_graph->set_h_axis_atrrs((const LATTICE_FONT_INFO*)c_theme::get_font(FONT_DEFAULT), x_axis_marks, H_AXIS_MARK_CNT);
	p_vitals_trend_graph->draw_h_axis();
	p_nibp_trend->set_h_axis_atrrs((const LATTICE_FONT_INFO*)c_theme::get_font(FONT_DEFAULT), x_axis_marks, H_AXIS_MARK_CNT);
	p_nibp_trend->draw_h_axis();

	//update data
	int data[7][MAX_TREND_DATA_CNT];//hr, spo2, rr, pr, sys, dia, mean
	long start_time = (time - (H_AXIS_MARK_CNT - 1) * H_AXIS_MARK_INTERVAL);
	read_trend_data(start_time, time,
					data[0], data[1], data[2], data[3], data[4], data[5], data[6],
					MAX_TREND_DATA_CNT);
	//draw vital trend
	p_vitals_trend_graph->draw_line(0, 0, data[0], MAX_TREND_DATA_CNT, HR_COLOR );
	p_vitals_trend_graph->draw_line(1, 1, data[1], MAX_TREND_DATA_CNT, SPO2_COLOR);
	p_vitals_trend_graph->draw_line(2, 2, data[2], MAX_TREND_DATA_CNT, RR_COLOR );
	//draw nibp trend
	p_nibp_trend->draw_line( 0, 0, data[4], MAX_TREND_DATA_CNT, NIBP_COLOR );
	p_nibp_trend->draw_line( 0, 1, data[5], MAX_TREND_DATA_CNT, NIBP_COLOR );
	p_nibp_trend->draw_line( 0, 2, data[6], MAX_TREND_DATA_CNT, NIBP_COLOR );
}

int c_trend_view::read_trend_data(long start_time, long end_time, int hr[], int spo2[], int rr[], int pr[],
						int nibp_sys[], int nibp_dia[], int nibp_mean[], int len)
{
	static VALUE_SET data[60];
	if ((0 >= len) || (len > (sizeof(data) / sizeof(VALUE_SET))))
	{
		ASSERT(false);
		return 0;
	}

	int read_cnt = c_database::get_instance()->read(start_time, end_time, data, len);
	if (read_cnt > len)
	{
		ASSERT(false);
	}

	long time = start_time;
	long detal = (end_time - start_time) / len;
	for (size_t i = 0; i < len; i++)
	{
		size_t j = 0;
		for (; j < read_cnt; j++)
		{
			if ((data[j].time >= time) && ((data[j].time - 60) <= time))
			{
				break;
			}
		}

		if (j == read_cnt)
		{
			hr[i] = rr[i] = spo2[i] = pr[i] = nibp_sys[i] = nibp_mean[i] = nibp_dia[i] = XXX;
		}
		else
		{
			hr[i] = data[j].hr;
			rr[i] = data[j].rr;
			spo2[i] = data[j].spo2;
			pr[i] = data[j].pr;
			nibp_sys[i] = data[j].nibp_sys;
			nibp_mean[i] = data[j].nibp_mean;
			nibp_dia[i] = data[j].nibp_dia;
		}
		time += detal;
	}
	return read_cnt;
}

typedef int(*SYNC_DATA)(int hr, int spo2, int rr, int nibp_sys, int nibp_dia, int nibp_mean);
extern SYNC_DATA gSyncData;
void c_trend_view::database_timer_callback(void* param)
{
	VALUE_SET data;
	memset(&data, 0, sizeof(data));
	c_value_manager* real_data_mgr = c_value_manager::get_instance();

	data.time = get_time_in_second();
	data.hr = data.pr = real_data_mgr->get_value(VALUE_HR);
	data.spo2 = real_data_mgr->get_value(VALUE_SPO2);
	data.rr = real_data_mgr->get_value(VALUE_RESP);
	data.nibp_sys = real_data_mgr->get_value(VALUE_NIBP_SYS);
	data.nibp_dia = real_data_mgr->get_value(VALUE_NIBP_DIA);
	data.nibp_mean = real_data_mgr->get_value(VALUE_NIBP_MEAN);
	c_database::get_instance()->write(data);

	((c_trend_view*)param)->update_trend_view();
	
	if (gSyncData)
	{
		gSyncData(data.hr, data.spo2, data.rr, data.nibp_sys, data.nibp_dia, data.nibp_mean);
	}
}

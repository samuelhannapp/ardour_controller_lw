#include <wx/wx.h>
#include <wx/splitter.h>
#include <wx/thread.h>
#include "OscSenderReceiver.hpp"
#include "wxOscReceiveThread.h"

#define SPACE_SIZE_SIDE 40
#define SPACE_SIZE 10
#define CHANNEL_NAME_HEIGHT 20 
#define MENU_SIZE 80
#define PANEL_OFFSET 400

class MyApp : public wxApp
{
public:
    bool OnInit() override;
};

wxIMPLEMENT_APP(MyApp);

#define CHANNEL_COUNT 8
#define PLUGIN_COUNT 8
/*
class OscThread : public wxThread {
public:
	OscThread(wxEvtHandler* handler);
	OscSenderReceiver* osc_controller;
protected:
	ExitCode Entry() override;
	
private:
	wxEvtHandler* m_handler;
};
*/


std::string file_name_scaling;

class MyFrame : public wxFrame
{
public:
    MyFrame(std::string udp_port_in, std::string udp_port_out);

private:
	//this is only for the plugin_list...
	int selected_channel = 0;
	void adjust_window(wxSize size);
	wxColor channel_not_selected_color;
	wxColor channel_selected_color;
	wxBoxSizer* channel_layout;
	std::vector<wxSizerItem*> spacers;
	wxSizerItem* top_spacer;
	wxBoxSizer* main_layout;

	std::string selected_plugin_name;

	wxStaticText* channel_name[CHANNEL_COUNT];
	wxStaticText* plugin_parameter_name[CHANNEL_COUNT];
	wxStaticText* knob_name[CHANNEL_COUNT];
	wxStaticText* send_name[CHANNEL_COUNT];
	wxStaticText* plugin_names[CHANNEL_COUNT][PLUGIN_COUNT];
	wxPanel* channel_name_panel[CHANNEL_COUNT];
	wxPanel* plugin_parameter_name_panel[CHANNEL_COUNT];
	wxPanel* knob_name_panel[CHANNEL_COUNT];
	wxPanel* send_name_panel[CHANNEL_COUNT];
	wxPanel* panel[CHANNEL_COUNT];
	wxPanel* plugin_names_panel_main[CHANNEL_COUNT];
	wxPanel* plugin_names_panel_individual[CHANNEL_COUNT];
	wxBoxSizer* plugin_names_sizer[CHANNEL_COUNT];

	int space_size_side = SPACE_SIZE_SIDE;
	int left_spacer_size = SPACE_SIZE;
	int panel_offset = PANEL_OFFSET;
    void OnHello(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
	void OnSize(wxSizeEvent& event);
public:
	void OnSlider(wxCommandEvent& event);
	void OnSlider_2(wxCommandEvent& event);
	void OnSlider_3(wxCommandEvent& event);

	void OnThreadUpdate(wxThreadEvent& event);

	wxOscReceiveThread* thread;
	OscSenderReceiver* osc_sender_receiver;
};

class WindowScalerFrame : public wxFrame {
public:
	WindowScalerFrame(MyFrame *parent, wxWindowID wxID_ANY, const wxString& title = "scale window", const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
	void SaveScaling(wxCommandEvent& event);
	void OpenScaling(wxCommandEvent& event);
	void OnSlider_1(wxCommandEvent& event);
	void OnSlider_2(wxCommandEvent& event);
	void OnSlider_3(wxCommandEvent& event);

	MyFrame *main_window;

	wxStaticText* slider_value_1;
	wxStaticText* slider_value_2;
	wxStaticText* slider_value_3;
	wxSlider* slider_1;
	wxSlider* slider_2;
	wxSlider* slider_3;
};

enum
{
    ID_Hello = 2, ID_OTHER, ID_THIRD, ID_PLUGIN
};



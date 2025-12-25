#include <wx/wx.h>
#include <wx/splitter.h>

#define SPACE_SIZE_SIDE 40
#define SPACE_SIZE 20
#define CHANNEL_NAME_HEIGHT 50 
#define MENU_SIZE 80
#define PANEL_OFFSET 400

class MyApp : public wxApp
{
public:
    bool OnInit() override;
};

wxIMPLEMENT_APP(MyApp);

#define CHANNEL_COUNT 8

class MyFrame : public wxFrame
{
public:
    MyFrame();

private:
	wxBoxSizer* channel_layout;
	std::vector<wxSizerItem*> spacers;
	wxSizerItem* top_spacer;
	wxBoxSizer* main_layout;

	wxStaticText* channel_name[CHANNEL_COUNT];
	wxPanel* channel_name_panel[CHANNEL_COUNT];
	wxPanel* panel[CHANNEL_COUNT];

	int space_size_side = SPACE_SIZE_SIDE;
	int left_spacer_size = SPACE_SIZE;
    void OnHello(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
	void OnSize(wxSizeEvent& event);
public:
	void OnSlider(wxCommandEvent& event);
	void OnSlider_2(wxCommandEvent& event);
};

class WindowScalerFrame : public wxFrame {
public:
	WindowScalerFrame(MyFrame *parent, wxWindowID wxID_ANY, const wxString& title = "scale window", const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
	void OnSlider(wxCommandEvent& event);
	wxStaticText* slider_value;
};

enum
{
    ID_Hello = 2, ID_OTHER, ID_THIRD, ID_PLUGIN
};

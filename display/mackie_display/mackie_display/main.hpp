#include <wx/wx.h>
#include <wx/splitter.h>

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
	wxBoxSizer* main_layout;

	wxStaticText* channel_name[CHANNEL_COUNT];
	wxPanel* channel_name_panel[CHANNEL_COUNT];
	wxPanel* panel[CHANNEL_COUNT];


    void OnHello(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
	void OnSize(wxSizeEvent& event);
};

enum
{
    ID_Hello = 2, ID_OTHER, ID_THIRD, ID_PLUGIN
};

#include "main.hpp"
#include "OscMessage.hpp"
#include "Defines.hpp"

bool MyApp::OnInit()
{
	MyFrame* frame = new MyFrame();
	frame->Show();
	return true;
}

MyFrame::MyFrame()
	: wxFrame(nullptr, wxID_ANY, "Special Purpose Controller (sp_controller)")
{
	main_layout = new wxBoxSizer(wxVERTICAL);
	fader_layout = new wxBoxSizer(wxHORIZONTAL);
	
	//this->m_sp_controller_plugin_name = std::string("bx_console SSL 4000 E");
	this->m_sp_controller_plugin_name = std::string("ACE Compressor (stereo)");
	
	this->previous_selected_plugin_name = std::string("nothing");

	for (int i = 0; i < CONTROLLER_SIZE; i++) {
		controller[i].fader = new wxSlider(this, wxID_ANY, 20, 0, 1000, wxDefaultPosition, wxSize(100, 30));
		controller[i].index = i + 1;
		controller[i].handler = (wxEvtHandler*)this;
		fader_layout->Add(controller[i].fader);
		controller[i].fader->Bind(wxEVT_SLIDER, &instance::OnSlider, &controller[i]);
	}
	main_layout->Add(fader_layout);
	this->SetSizerAndFit(main_layout);
	this->osc_sender_receiver = new OscSenderReceiver("127.0.0.1", 17, 3819);
	this->receive_thread = new wxOscReceiveThread(this, this->osc_sender_receiver);
	receive_thread->Run();
	Bind(wxEVT_THREAD, &MyFrame::OnThreadUpdate, this);

	this->plugin_multiplexer = new plugin_multiplexer_struct;
	this->plugin_multiplexer->initialize_plugin_multiplexer();
	this->plugin_multiplexer->initialize_plugin_multiplexer_from_controller_and_from_plugin();
	this->plugin_multiplexer->setup(this->m_sp_controller_plugin_name);

	OscMessage setup_msg("/set_surface");
	setup_msg.PushInt(1);
	setup_msg.PushInt((1 << AudioTracks) | (1 << MidiTracks));
	setup_msg.PushInt(1 << ExtraSelectOnlyFeedback);
	setup_msg.PushInt(0b1000);

	this->osc_sender_receiver->send_data(setup_msg);
}

void instance::OnSlider(wxCommandEvent& event)
{
	OscMessage osc_message("/wxSlider");
	//thread_message message;

	osc_message.PushInt(this->index + (bank * CONTROLLER_SIZE));
	int value = event.GetInt();
	float value_float = float(value) / 1000.0;
	osc_message.PushFloat(value_float);

	int size = 0;
	char* array;
	array = osc_message.GetBytes(size);
	OscMessage osc_message_2(array, size);

	wxThreadEvent event_1 = wxThreadEvent(wxEVT_THREAD); // No specific id
	event_1.SetPayload(osc_message_2);
	wxQueueEvent(handler, event_1.Clone());
}


//I just realised the plugin selected in the /select/plugin commands is not global!
//each controller has it's own index


void MyFrame::OnThreadUpdate(wxThreadEvent& event)
{
	// SAFE: runs on GUI thread
	OscMessage osc_message = event.GetPayload<OscMessage>();

	//std::cout << osc_message.GetAddress();

	if (!osc_message.GetAddress().compare("/select/plugin/parameter/name")) {
		int plugin_parameter_id = osc_message.get_int(0);
		std::string plugin_parameter_name = osc_message.get_string(1);

		if (plugin_parameter_id >= this->plugin_multiplexer->plugin_multiplexer_from_plugin.size())
			return;

		int plugin_parameter_id_routed = this->plugin_multiplexer->plugin_multiplexer_from_plugin[plugin_parameter_id];

		if (plugin_parameter_id_routed >= MAX_PLUGIN_PARAMETERS)
			return;

		if (plugin_parameter_id_routed <= 0)
			return;
	}
	else if (!osc_message.GetAddress().compare("/select/plugin/parameter")) {
		int plugin_parameter_id = osc_message.get_int(0);
		float plugin_parameter_value = osc_message.initialize_type_list().at(1) == 'f' ? osc_message.get_float(1) : osc_message.get_double(1);

		//if the selected plugin is not sp_controller plugin
		if(this->selected_plugin_name.compare(this->m_sp_controller_plugin_name))
			return;

		if (plugin_parameter_id >= this->plugin_multiplexer->plugin_multiplexer_from_plugin.size())
			return;

		int plugin_parameter_id_routed = this->plugin_multiplexer->plugin_multiplexer_from_plugin[plugin_parameter_id];

		if (plugin_parameter_id_routed <= 0)
			return;

		if ((plugin_parameter_id_routed / CONTROLLER_SIZE) == bank)
			this->controller[(plugin_parameter_id_routed - 1) % CONTROLLER_SIZE].fader->SetValue(plugin_parameter_value * 1000);
	}
	else if (!osc_message.GetAddress().compare("/wxSlider")) {
		int index = osc_message.get_int(0);
		float value = (float)osc_message.get_float(1);
		int plugin_parameter_id_routed = this->plugin_multiplexer->plugin_multiplexer_from_controller[index + CONTROLLER_SIZE * bank];

		if ((this->m_sp_controller_plugin_name.compare(this->selected_plugin_name))) //if there is not the sp_controller_plugin in the strip
			return;

		OscMessage osc_message_to_ardour("/select/plugin/parameter");
		osc_message_to_ardour.PushInt(plugin_parameter_id_routed);
		osc_message_to_ardour.PushFloat(value);
		int size = 0;
		osc_message.GetBytes(size);
		this->osc_sender_receiver->send_data(osc_message_to_ardour);
	}
	else if (!osc_message.GetAddress().compare("/select/plugin/name")) {
		std::string plugin_name = osc_message.get_string(0);

		if (plugin_name.size() == 1)
			return;

		this->selected_plugin_name = plugin_name;

		if (!this->m_sp_controller_plugin_name.compare(plugin_name)) { //the right plugin is selected
			this->plugin_name_before_increment = std::string("nothing");
			return;
		}
		
		if (!this->plugin_name_before_increment.compare(plugin_name)) //if there is only the one plugin in the strip, this is flawed...
			return;
		this->plugin_name_before_increment = plugin_name;

		OscMessage plugin_up_message("/select/plugin");
		plugin_up_message.PushInt(1);
		this->osc_sender_receiver->send_data(plugin_up_message);

		this->previous_selected_plugin_name = plugin_name;
	}
	
	return;
}
#include "NodeFactory.h"
#ifdef pi
#define __CVEDIX_UNDEF_PI
#undef pi
#endif
#ifdef __CVEDIX_UNDEF_PI
#define pi 3.1415926
#undef __CVEDIX_UNDEF_PI
#endif
#include <mutex>
#include <memory>
#include <string>
#include <sstream>

void MakeNode::NodeRegister(json &jsonData)
{

// (MQTT broker node class moved to mqtt branch)

    for (int countNode = 0; countNode < jsonData["nodes"].size(); countNode++)
    {
        Node = jsonData["nodes"][countNode]["type"].get<std::string>();
        std::cout << "[INFO] Registering node: " << Node << std::endl;
        //----------------------------------------------------------------------------------------------------------------
        /*
                            _____  _____ _   _______  _____  _____
                            /  ___||  _  | | | | ___ \/  __ \|  ___|
                            \ `--. | | | | | | | |_/ /| /  \/| |__
                            `--. \ | | | | | | |    / | |    |  __|
                            /\__/ /\ \_/ / |_| | |\ \ | \__/\| |___
                            \____/  \___/ \___/\_| \_| \____/\____/
        */
        if (Node == "cvedix_file_src_node")
        {
            file_src = std::make_shared<cvedix_nodes::cvedix_file_src_node>(
                jsonData["nodes"][countNode]["config"]["node_name"].get<std::string>(),
                jsonData["nodes"][countNode]["config"]["channel_index"].get<int>(),
                "/opt/cvedix/media/" + jsonData["nodes"][countNode]["config"]["video_name"].get<std::string>(),
                jsonData["nodes"][countNode]["config"]["resize_ratio"].get<float>(),
                true
                );
            nodes.push_back(file_src);
        }

        //----------------------------------------------------------------------------------------------------------------
        //----------------------------------------------------------------------------------------------------------------
        /*
                                              _______ _____            _____ _  __
                                             |__   __|  __ \     /\   / ____| |/ /
                                                | |  | |__) |   /  \ | |    | ' /
                                                | |  |  _  /   / /\ \| |    |  <
                                                | |  | | \ \  / ____ \ |____| . \
                                                |_|  |_|  \_\/_/    \_\_____|_|\_\
        */
        else if (Node == "cvedix_ocsort_track_node")
        {
            ocsort_track = std::make_shared<cvedix_nodes::cvedix_ocsort_track_node>(
                jsonData["nodes"][countNode]["config"]["node_name"].get<std::string>(),
                cvedix_nodes::cvedix_track_for::NORMAL,
                0.5,
                15,
                3,
                0.3,
                3,
                "iou",
                0.2,
                true
            );
            nodes.push_back(ocsort_track);
        }
        else if (Node == "cvedix_bytetrack_node")
        {
            bytetrack = std::make_shared<cvedix_nodes::cvedix_bytetrack_node>(
                jsonData["nodes"][countNode]["config"]["node_name"].get<std::string>(),
                cvedix_nodes::cvedix_track_for::NORMAL,
                0.5,
                0.9,
                0.6,
                20,
                15
            );
            nodes.push_back(bytetrack);
        }
        //----------------------------------------------------------------------------------------------------------------
        //----------------------------------------------------------------------------------------------------------------
        /*
                                             _____ _   _ ______ ______ _____   _____
                                            |_   _| \ | |  ____|  ____|  __ \/ ____|
                                              | | |  \| | |__  | |__  | |__) | (___
                                              | | | . ` |  __| |  __| |  _  / \___ \
                                              | |_| |\  | |    | |____| | \ \ ____) |
                                            |_____|_| \_|_|    |______|_|  \_\_____/

        */

        else if (Node == "cvedix_yolo_detector_node")
        {
            auto yolo_detector = std::make_shared<cvedix_nodes::cvedix_yolo_detector_node>(
                jsonData["nodes"][countNode]["config"]["node_name"].get<std::string>(),
                "/opt/cvedix/model/yolov3-tiny-2022-0721_best.weights",
                "/opt/cvedix/model/yolov3-tiny-2022-0721.cfg",
                "/opt/cvedix/model/yolov3_tiny_5classes.txt"
            );
            nodes.push_back(yolo_detector);
        }
        // ----------------------------------------------------------------------------------------------------------------
        // ----------------------------------------------------------------------------------------------------------------
        /*                                          ____
                                                    |  _ \   /\
                                                    | |_) | /  \
                                                    |  _ < / /\ \
                                                    | |_) / ____ \
                                                    |____/_/    \_\
        */
        else if (Node == "cvedix_ba_line_crossline_node")
        {
            auto &cfg = jsonData["nodes"][countNode]["config"];
            int channel_index = 0;
            if (cfg.contains("channel_index"))
                channel_index = cfg["channel_index"].get<int>();

            std::map<int, std::vector<cvedix_objects::cvedix_line>> multi_lines;

            if (cfg.contains("lines") && cfg["lines"].is_array())
            {
                std::vector<cvedix_objects::cvedix_line> v;
                for (auto &ln : cfg["lines"])
                {
                    int lx1 = ln.value("x1", 0);
                    int ly1 = ln.value("y1", 0);
                    int lx2 = ln.value("x2", 0);
                    int ly2 = ln.value("y2", 0);
                    v.emplace_back(cvedix_objects::cvedix_point(lx1, ly1), cvedix_objects::cvedix_point(lx2, ly2));
                }
                multi_lines[channel_index] = std::move(v);
            }
            else if (cfg.contains("channels") && cfg["channels"].is_object())
            {
                // support channels: { "0": [ {x1,y1,x2,y2}, ... ], "1": [...] }
                for (auto it = cfg["channels"].begin(); it != cfg["channels"].end(); ++it)
                {
                    int ch = std::stoi(it.key());
                    std::vector<cvedix_objects::cvedix_line> v;
                    if (it.value().is_array())
                    {
                        for (auto &ln : it.value())
                        {
                            int lx1 = ln.value("x1", 0);
                            int ly1 = ln.value("y1", 0);
                            int lx2 = ln.value("x2", 0);
                            int ly2 = ln.value("y2", 0);
                            v.emplace_back(cvedix_objects::cvedix_point(lx1, ly1), cvedix_objects::cvedix_point(lx2, ly2));
                        }
                    }
                    multi_lines[ch] = std::move(v);
                }
            }
            else
            {
                // legacy single-line config
                int x1 = cfg.value("x1", 0);
                int y1 = cfg.value("y1", 0);
                int x2 = cfg.value("x2", 0);
                int y2 = cfg.value("y2", 0);
                std::vector<cvedix_objects::cvedix_line> v;
                v.emplace_back(cvedix_objects::cvedix_point(x1, y1), cvedix_objects::cvedix_point(x2, y2));
                multi_lines[channel_index] = std::move(v);
            }

            ba_line_crossline = std::make_shared<cvedix_nodes::cvedix_ba_line_crossline_node>(
                jsonData["nodes"][countNode]["config"]["node_name"].get<std::string>(),
                multi_lines
            );
            nodes.push_back(ba_line_crossline);
        }
    
        else if (Node == "cvedix_ba_area_enter_exit_node")
        {
            auto &cfg = jsonData["nodes"][countNode]["config"];

            std::map<int, std::vector<std::vector<cvedix_objects::cvedix_point>>> areas;
            std::map<int, std::vector<cvedix_nodes::area_alert_config>> configs;

            int default_channel = cfg.value("channel_index", 0);

            // Parse per-channel areas: either `channels` (object) or top-level `areas` (array)
            if (cfg.contains("channels") && cfg["channels"].is_object())
            {
                for (auto it = cfg["channels"].begin(); it != cfg["channels"].end(); ++it)
                {
                    int ch = std::stoi(it.key());
                    if (!it.value().is_array())
                        continue;
                    std::vector<std::vector<cvedix_objects::cvedix_point>> ch_areas;
                    for (auto &area : it.value())
                    {
                        if (!area.is_array())
                            continue;
                        std::vector<cvedix_objects::cvedix_point> poly;
                        for (auto &pt : area)
                        {
                            int x = pt.value("x", 0);
                            int y = pt.value("y", 0);
                            poly.emplace_back(cvedix_objects::cvedix_point(x, y));
                        }
                        ch_areas.push_back(std::move(poly));
                    }
                    areas[ch] = std::move(ch_areas);
                }
            }
            else if (cfg.contains("areas") && cfg["areas"].is_array())
            {
                std::vector<std::vector<cvedix_objects::cvedix_point>> ch_areas;
                for (auto &area : cfg["areas"]) {
                    if (!area.is_array())
                        continue;
                    std::vector<cvedix_objects::cvedix_point> poly;
                    for (auto &pt : area) {
                        int x = pt.value("x", 0);
                        int y = pt.value("y", 0);
                        poly.emplace_back(cvedix_objects::cvedix_point(x, y));
                    }
                    ch_areas.push_back(std::move(poly));
                }
                areas[default_channel] = std::move(ch_areas);
            }

            // Parse area alert configs if provided; fallback to default configs per-area
            if (cfg.contains("area_configs") && cfg["area_configs"].is_object())
            {
                for (auto it = cfg["area_configs"].begin(); it != cfg["area_configs"].end(); ++it)
                {
                    int ch = std::stoi(it.key());
                    std::vector<cvedix_nodes::area_alert_config> ch_cfgs;
                    if (it.value().is_array())
                    {
                        for (auto &a : it.value())
                        {
                            bool enter = a.value("enter_alert", true);
                            bool exit = a.value("exit_alert", true);
                            std::string name = a.value("name", std::string());
                            cv::Scalar color(0, 220, 0);
                            if (a.contains("color") && a["color"].is_array() && a["color"].size() >= 3)
                            {
                                int b = a["color"][0].get<int>();
                                int g = a["color"][1].get<int>();
                                int r = a["color"][2].get<int>();
                                color = cv::Scalar(b, g, r);
                            }
                            ch_cfgs.emplace_back(enter, exit, name, color);
                        }
                    }
                    configs[ch] = std::move(ch_cfgs);
                }
            }

            // If no explicit configs provided, create sensible defaults matching number of areas
            for (auto &p : areas)
            {
                int ch = p.first;
                if (configs.find(ch) != configs.end())
                    continue;
                std::vector<cvedix_nodes::area_alert_config> vcfg;
                for (size_t i = 0; i < p.second.size(); ++i)
                {
                    std::string name = std::string("Area") + std::to_string(i);
                    vcfg.emplace_back(true, true, name, cv::Scalar(0, 220, 0));
                }
                configs[ch] = std::move(vcfg);
            }

            bool report_enter = cfg.value("report_enter", false);
            bool report_exit = cfg.value("report_exit", false);

            ba_area_enter_exit = std::make_shared<cvedix_nodes::cvedix_ba_area_enter_exit_node>(
                jsonData["nodes"][countNode]["config"]["node_name"].get<std::string>(),
                areas,
                configs,
                report_enter,
                report_exit
            );
            nodes.push_back(ba_area_enter_exit);
        }
        /*----------------------------------------------------------------------------------------------------------------
        /*----------------------------------------------------------------------------------------------------------------
                                            _____ ___________
                                            |  _  /  ___|  _  \
                                            | | | \ `--.| | | |
                                            | | | |`--. \ | | |
                                            \ \_/ /\__/ / |/ /
                                             \___/\____/|___/
       */
        else if (Node == "cvedix_ba_line_crossline_osd_node")
        {
            ba_line_crossline_osd = std::make_shared<cvedix_nodes::cvedix_ba_line_crossline_osd_node>(
                jsonData["nodes"][countNode]["config"]["node_name"].get<std::string>()
                );
            nodes.push_back(ba_line_crossline_osd);
        }
        else if (Node == "cvedix_ba_area_enter_exit_osd_node")
        {
            ba_area_enter_exit_osd = std::make_shared<cvedix_nodes::cvedix_ba_area_enter_exit_osd_node>(
                jsonData["nodes"][countNode]["config"]["node_name"].get<std::string>()
                );
            nodes.push_back(ba_area_enter_exit_osd);
        }
        
        //----------------------------------------------------------------------------------------------------------------
        //----------------------------------------------------------------------------------------------------------------

        /*
                                ______ _____ _____ _________________
                                | ___ \  ___/  __ \  _  | ___ \  _  \
                                | |_/ / |__ | /  \/ | | | |_/ / | | |
                                |    /|  __|| |   | | | |    /| | | |
                                | |\ \| |___| \__/\ \_/ / |\ \| |/ /
                                \_| \_\____/ \____/\___/\_| \_|___/
        */
        //----------------------------------------------------------------------------------------------------------------
        //----------------------------------------------------------------------------------------------------------------
        /*
                                    ____________ _____ _   __ ___________
                                    | ___ \ ___ \  _  | | / /|  ___| ___ \
                                    | |_/ / |_/ / | | | |/ / | |__ | |_/ /
                                    | ___ \    /| | | |    \ |  __||    /
                                    | |_/ / |\ \\ \_/ / |\  \| |___| |\ \
                                    \____/\_| \_|\___/\_| \_/\____/\_| \_|
        */
        else if (Node == "cvedix_mqtt_broker_node")
        {
            auto &cfg = jsonData["nodes"][countNode]["config"];
            std::string broker = cfg.value("broker", std::string("127.0.0.1"));
            int port = cfg.value("port", 1883);
            std::string topic = cfg.value("topic", std::string("events"));

            // Inline broker class placed here so all MQTT logic is inside this branch
            class factory_mqtt_broker_node : public cvedix_nodes::cvedix_json_enhanced_console_broker_node {
            private:
                std::unique_ptr<cvedix_utils::cvedix_mqtt_client> mqtt_client_;
                std::mutex publish_mutex_;
                std::string topic_;

                virtual void format_msg(const std::shared_ptr<cvedix_objects::cvedix_frame_meta>& meta, std::string& msg) override {
                    try {
                        if (meta->ba_results.empty()) { msg.clear(); return; }
                        std::stringstream ss;
                        ss << "{\"frame_id\":" << meta->frame_index << ",\"ba_count\":" << meta->ba_results.size() << "}";
                        msg = ss.str();
                    } catch (...) { msg.clear(); }
                }

                virtual void broke_msg(const std::string& msg) override {
                    if (msg.empty()) return;
                    std::lock_guard<std::mutex> lk(publish_mutex_);
                    if (mqtt_client_ && mqtt_client_->is_ready()) {
                        try { mqtt_client_->publish(topic_, msg, 1, false); }
                        catch (...) {}
                    }
                }

            public:
                factory_mqtt_broker_node(const std::string &name, const std::string &broker, int port, const std::string &topic)
                    : cvedix_nodes::cvedix_json_enhanced_console_broker_node(name, cvedix_nodes::cvedix_broke_for::NORMAL, 100, 500, false),
                      topic_(topic)
                {
                    mqtt_client_ = std::make_unique<cvedix_utils::cvedix_mqtt_client>(broker, port, name + "_client", 60);
                    mqtt_client_->set_auto_reconnect(true, 5000);
                    mqtt_client_->connect("", "");
                }

                ~factory_mqtt_broker_node() {
                    try { if (mqtt_client_) mqtt_client_->disconnect(); } catch (...) {}
                }
            };

            auto mqtt_node = std::make_shared<factory_mqtt_broker_node>(
                jsonData["nodes"][countNode]["config"]["node_name"].get<std::string>(),
                broker, port, topic);
            nodes.push_back(mqtt_node);
        }
        //----------------------------------------------------------------------------------------------------------------
        //----------------------------------------------------------------------------------------------------------------
        /*
                                                                ___  ______________
                                                                |  \/  |_   _|  _  \
                                                                | .  . | | | | | | |
                                                                | |\/| | | | | | | |
                                                                | |  | |_| |_| |/ /
                                                                \_|  |_/\___/|___/

        */
        
        //----------------------------------------------------------------------------------------------------------------

        //----------------------------------------------------------------------------------------------------------------
        /*
                                                ______ _____ _____
                                                |  _  \  ___/  ___|
                                                | | | | |__ \ `--.
                                                | | | |  __| `--. \
                                                | |/ /| |___/\__/ /
                                                |___/ \____/\____/
        */
        else if (Node == "cvedix_rtmp_des_node")
        {
            auto rtmp_des = std::make_shared<cvedix_nodes::cvedix_rtmp_des_node>(
                jsonData["nodes"][countNode]["config"]["node_name"].get<std::string>(),
                jsonData["nodes"][countNode]["config"]["channel_index"].get<int>(),
                jsonData["nodes"][countNode]["config"]["rtmp_url"].get<std::string>(),
                cvedix_objects::cvedix_size{
                    jsonData["nodes"][countNode]["config"]["resolution_width"].get<int>(),
                    jsonData["nodes"][countNode]["config"]["resolution_height"].get<int>()
                }
            );

            nodes.push_back(rtmp_des);
            des_node++;
        }
        else if (Node == "cvedix_screen_des_node")
        {
            auto screen_des = std::make_shared<cvedix_nodes::cvedix_screen_des_node>(
                jsonData["nodes"][countNode]["config"]["node_name"].get<std::string>(),
                jsonData["nodes"][countNode]["config"]["channel_index"].get<int>(),
                true);
            nodes.push_back(screen_des);
            des_node++;
        }
    }

    //----------------------------------------------------------------------------------------------------------------
}




//----------------------------------------------------------------------------------------------------------------------

                

//--------------------------------------------------------------------------------------------------------------------



void MakeNode::MatchingPipeline()
{
    if (nodes.empty())
        return;

    int des_node_pointer = static_cast<int>(nodes.size()) - des_node;
    std::cout << "[DEBUG] MatchingPipeline: nodes.size()=" << nodes.size()
              << " des_node=" << des_node
              << " des_node_pointer=" << des_node_pointer << std::endl;

    for (int i = 0; i < static_cast<int>(nodes.size()); i++)
    {
        // link source/process nodes in linear order up to the first destination
        if (i < des_node_pointer - 1)
        {
            std::cout << "[DEBUG] attach: " << (i + 1) << " <- " << i << std::endl;
            nodes[i + 1]->attach_to({nodes[i]});
        }
        // link destination nodes (those at or after des_node_pointer) to the last process node
        else if (i >= des_node_pointer)
        {
            if (des_node_pointer - 1 >= 0)
            {
                std::cout << "[DEBUG] attach dest: " << i << " <- " << (des_node_pointer - 1) << std::endl;
                nodes[i]->attach_to({nodes[des_node_pointer - 1]});
            }
        }
        // when i == des_node_pointer -1 -> bridge node, already linked from previous iteration
    }
}

void MakeNode::StartPipeline()
{
    auto src_node = std::dynamic_pointer_cast<cvedix_nodes::cvedix_src_node>(nodes[0]);
    std::cout << "[INFO] Starting pipeline..." << std::endl;
    src_node->start();
    std::cout << "[INFO] Pipeline started successfully" << std::endl;
    // cvedix_utils::cvedix_analysis_board board({nodes[0]});
    // board.display(1, true);
}

void MakeNode::StopPipeline()
{
    auto src_node = std::dynamic_pointer_cast<cvedix_nodes::cvedix_src_node>(nodes[0]);
    std::cout << "[INFO] Stopping pipeline..." << std::endl;
    for (int i = nodes.size() - 1; i >= 0; i--)
    {
        nodes[i]->detach();
    }
    src_node->stop();
    std::cout << "[INFO] Pipeline stopped successfully" << std::endl;
}

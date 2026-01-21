#include "GetImageList.h"

void GetImageList::asyncHandleHttpRequest(const HttpRequestPtr &req, std::function<void(const HttpResponsePtr &)> &&callback)
{
    json resp;
    resp["image"] = json::array();
    std::string list_path = "/opt/cvedix/image";
    for (std::filesystem::directory_iterator it(list_path);; ++it)
    {
        json item;
        if (it == std::filesystem::directory_iterator{})
            break;
        const auto &entry = *it;
        item["name"] = entry.path().filename().string();
        item["size"] = std::filesystem::file_size(entry.path());
        resp["image"].push_back(item);
    }
    auto httpResp = HttpResponse::newHttpResponse();
    httpResp->setContentTypeCode(CT_APPLICATION_JSON);
    httpResp->setBody(resp.dump());
    callback(httpResp);
}

void DeleteImage::asyncHandleHttpRequest(const HttpRequestPtr &req, std::function<void(const HttpResponsePtr &)> &&callback)
{
    json jsonData = json::parse(req->body());
    std::string image_name = jsonData.value("image_name", "");
    if (image_name == "1.jpg" || image_name == "2.jpg" || image_name == "3.jpg" || image_name == "4.jpg")
    {
        json res;
        res["status"] = "ERROR";
        res["message"] = "Cannot delete default models";
        auto httpResp = HttpResponse::newHttpResponse();
        httpResp->setContentTypeCode(CT_APPLICATION_JSON);
        httpResp->setBody(res.dump());
        callback(httpResp);
        return;
    }
    std::string image_path = "/opt/cvedix/image/" + image_name;
    if (std::filesystem::exists(image_path))
    {
        std::filesystem::remove(image_path);
        json res;
        res["status"] = "SUCCESS";
        res["message"] = "Image deleted successfully";
        auto httpResp = HttpResponse::newHttpResponse();
        httpResp->setContentTypeCode(CT_APPLICATION_JSON);
        httpResp->setBody(res.dump());
        callback(httpResp);
    }
}
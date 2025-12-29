#pragma once

#include <drogon/HttpSimpleController.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <filesystem>
#include <fstream>

using namespace drogon;
using json = nlohmann::json;

class GetImageList : public drogon::HttpSimpleController<GetImageList>
{
public:
  void asyncHandleHttpRequest(const HttpRequestPtr &req, std::function<void(const HttpResponsePtr &)> &&callback) override;
  PATH_LIST_BEGIN
  // list path definitions here;
  PATH_ADD("/lego/getimagelist", Get);
  PATH_LIST_END
};

class DeleteImage : public drogon::HttpSimpleController<DeleteImage>
{
public:
  void asyncHandleHttpRequest(const HttpRequestPtr &req, std::function<void(const HttpResponsePtr &)> &&callback) override;
  PATH_LIST_BEGIN
  PATH_ADD("/lego/deleteimage", Post);
  PATH_LIST_END
};
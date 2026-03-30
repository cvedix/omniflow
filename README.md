# Omniflow

Một giải pháp nền tảng low-code giúp kết nối công nghệ AI (như các thuật toán học sâu) với các ứng dụng thực tế trong công nghiệp. Nền tảng này giúp đơn giản hóa việc tích hợp các hệ thống như Robot Operating System (ROS), cho phép người dùng thiết kế quy trình làm việc bằng ngôn ngữ lập trình dạng khối, giảm thiểu nhu cầu lập trình thủ công.

Một số đặc điểm nổi bật của CvedixLego:

- Giao diện kéo-thả lập trình hóa các quy trình xử lý.
- Kết nối được với nhiều thiết bị ngoại vi như Modbus/TCP, Arduino I/O, cổng Serial, camera IP, webcam...
- Lưu trữ, chú thích dữ liệu phục vụ huấn luyện AI.
- Hỗ trợ huấn luyện mô hình AI bằng cả CPU/GPU.
- Truyền dữ liệu tới các hệ thống khác thông qua MQTT hoặc REST API, hoặc phát trực tuyến bằng RTSP.
- Tạo giao diện người dùng cơ bản với các thành phần như nút bấm, hình ảnh, chữ led.
- Thiết kế các quy trình xử lý song song hoặc tuần tự.
- Tương thích đa hệ điều hành, dễ dàng sử dụng cho nhiều đối tượng.

Tóm lại, CvedixLego là nền tảng công nghệ cốt lõi giúp doanh nghiệp và ngành công nghiệp triển khai, ứng dụng AI vào tự động hóa, nhận diện và điều khiển thiết bị một cách dễ dàng và hiệu quả.

---

## 🏗️ Build & Run

### Build backend


```bash
cd build
cmake ..
make -j4
```





### Chạy server
```bash
./cvedix_backend # hoặc bất cứ project name nào gán lúc build cmake
```








### Test với curl
```bash
# GET request
curl http://localhost:5555/api/hello

# POST JSON
curl -X POST http://localhost:5555/api/data \
  -H "Content-Type: application/json" \
  -d '{"key":"value"}'

# File upload
curl -X POST http://localhost:5555/api/upload \
  -F "file=@image.jpg"
```

---

### Test với postman
```postman JSON
# POST JSON API: 127.0.0.1:8090/api/PipelineNode
# POST JSON
{
    "nodes": [
        {
            "type": "cvedix_file_src_node",
            "config": {
                "node_name": "file_src",
                "channel_index": 0,
                "resize_ratio": 1.0,
                "cycle": true,
                "gst_decoder_name": "avdec_h264",
                "skip_interval": 0
            }
        },
        {
            "type": "cvedix_yunet_face_detector_node",
            "config": {
                "node_name": "face_detector"
            }
        },
        {
            "type": "cvedix_face_osd_node",
            "config": {
                "node_name": "osd"
            }
        },
        {
            "type": "cvedix_screen_des_node",
            "config": {
                "node_name": "screen_des_0",
                "channel_index": 0,   
                "osd": true
            }
        },
        {
            "type": "cvedix_rtmp_des_node",
            "config": {
                "node_name": "rtmp_node",
                "channel_index": 0,
                "rtmp_url": "rtmp://anhoidong.datacenter.cvedix.com:1935/live/stream",
                "resolution_width": 960,
                "resolution_height": 540,
                "bitrate": 1800,
                "osd": true,
                "gst_encoder_name": "x264enc"
            }
        }
    ]
}


# POST VIDEO API: 127.0.0.1:8090/api/PostVideo

**📌 Lưu ý:**
Định dạng video .mp4
Short Video

---
```

### Build Front-End

1. navigate vào thư mục FrontEnd vả cài các dependencies cần thiết

```bash
cd frontend
npm i
```

2. Khởi tạo 1 file .env và copy nội dung cần thiết từ .env_example qua. Có thể xem qua .env này cho URL backend, HLS và RTMP

```bash
# Backend API Configuration
VITE_API_BASE_URL=http://localhost:8090

# RTMP Stream Configuration
VITE_DEFAULT_RTMP_URL=rtmp://127.0.0.1:1935/live/test

# HLS Media Server Configuration
# The backend streams RTMP to the RTMP server, and the HLS server transcodes it for browser playback
VITE_HLS_BASE_URL=https://spatial-transcripts-ddr-compound.trycloudflare.com
```

3. Chạy Frontend

```bash
npm run dev
```
4. Truy cập [localhost:5173](http://localhost:5173/)

### Khởi tạo luồng RTMP và HLS local (nếu cần)

1. Đảm bảo việc Docker zlmedia có đang chạy ổn định

```
docker start zlmediakit
docker ps
```

2. Kiểm tra response của  zlmedia

```
curl http://127.0.0.1:8080/index/api/getMediaList?secret=123456

```

3. Chạy cloudfare để đẩy lên HLS server sau khi có luồng RTMP:

```
cloudflared tunnel --url http://127.0.0.1:8080\
```

**lưu ý, sau khi chạy, cloudfare tunnel sẽ trả lại 1 domain HLS, sử dụng domain đó cho .env của FrontEnd







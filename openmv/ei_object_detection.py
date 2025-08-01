# Edge Impulse - OpenMV FOMO Object Detection Example
#
# 功能：仅在指定区域内（y:45-180, x:30-215）检测动物，连续两次且中心位置在15像素内时，通过串口1打印动物名称
# 支持同时处理多种动物的检测
#
# This work is licensed under the MIT license.
# Copyright (c) 2013-2024 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE

import sensor, image, time, ml, math, uos, gc, pyb

# 初始化串口1用于输出结果 (TX=P4, RX=P5 on OpenMV H7)
uart = pyb.UART(1, 115200, timeout_char=1000)

# 设置像素死区范围（只关注此范围内的检测）
DEAD_ZONE_X_MIN = 35
DEAD_ZONE_X_MAX = 205
DEAD_ZONE_Y_MIN = 35
DEAD_ZONE_Y_MAX = 205

sensor.reset()                         # 重置并初始化传感器
sensor.set_pixformat(sensor.RGB565)    # 设置像素格式为RGB565
sensor.set_framesize(sensor.QVGA)      # 设置帧大小为QVGA (320x240)
sensor.set_windowing((240, 240))       # 设置240x240窗口
sensor.skip_frames(time=2000)          # 让摄像头调整

net = None
labels = None
min_confidence = 0.5

# 跟踪变量：为每个类别维护独立的检测历史
# 格式: {label_index: {
#           'positions': [(x1, y1), (x2, y2)],  # 最近2次位置
#           'notified': False  # 是否已发送通知
#       }, ...}
tracking_data = {}

try:
    # 加载模型
    net = ml.Model("trained.tflite", load_to_fb=uos.stat('trained.tflite')[6] > (gc.mem_free() - (64*1024)))
except Exception as e:
    raise Exception('Failed to load "trained.tflite", did you copy the .tflite and labels.txt file? (' + str(e) + ')')

try:
    labels = [line.rstrip('\n') for line in open("labels.txt")]
    # 初始化每个类别的跟踪数据
    for i in range(len(labels)):
        tracking_data[i] = {
            'positions': [],
            'notified': False
        }
except Exception as e:
    raise Exception('Failed to load "labels.txt", did you copy the files? (' + str(e) + ')')

colors = [ # 颜色列表，用于不同类别的标记
    (255,   0,   0),
    (  0, 255,   0),
    (255, 255,   0),
    (  0,   0, 255),
    (255,   0, 255),
    (  0, 255, 255),
    (255, 255, 255),
]

threshold_list = [(math.ceil(min_confidence * 255), 255)]

def fomo_post_process(model, inputs, outputs):
    ob, oh, ow, oc = model.output_shape[0]

    x_scale = inputs[0].roi[2] / ow
    y_scale = inputs[0].roi[3] / oh

    scale = min(x_scale, y_scale)

    x_offset = ((inputs[0].roi[2] - (ow * scale)) / 2) + inputs[0].roi[0]
    y_offset = ((inputs[0].roi[3] - (ow * scale)) / 2) + inputs[0].roi[1]

    l = [[] for i in range(oc)]

    for i in range(oc):
        img = image.Image(outputs[0][0, :, :, i] * 255)
        blobs = img.find_blobs(
            threshold_list, x_stride=1, y_stride=1, area_threshold=1, pixels_threshold=1
        )
        for b in blobs:
            rect = b.rect()
            x, y, w, h = rect
            score = (
                img.get_statistics(thresholds=threshold_list, roi=rect).l_mean() / 255.0
            )
            x = int((x * scale) + x_offset)
            y = int((y * scale) + y_offset)
            w = int(w * scale)
            h = int(h * scale)
            l[i].append((x, y, w, h, score))
    return l

def is_inside_detection_zone(x, y):
    """检查目标中心是否在检测区域内（非死区）"""
    return (DEAD_ZONE_X_MIN <= x <= DEAD_ZONE_X_MAX and
            DEAD_ZONE_Y_MIN <= y <= DEAD_ZONE_Y_MAX)

def check_position_consistency(points, threshold=15):
    """检查一组点是否都在阈值范围内"""
    if len(points) < 2:
        return False

    # 计算所有点与第一个点的距离
    x0, y0 = points[0]
    for (x, y) in points[1:]:
        # 计算欧氏距离
        distance = math.sqrt((x - x0)**2 + (y - y0)** 2)
        if distance > threshold:
            return False
    return True

def reset_tracking_data(label_index):
    """重置指定类别的跟踪数据"""
    tracking_data[label_index]['positions'] = []
    tracking_data[label_index]['notified'] = False

clock = time.clock()
while(True):
    clock.tick()
    img = sensor.snapshot()

    # 绘制检测区域边界（便于调试）
    img.draw_rectangle((DEAD_ZONE_X_MIN, DEAD_ZONE_Y_MIN,
                       DEAD_ZONE_X_MAX - DEAD_ZONE_X_MIN,
                       DEAD_ZONE_Y_MAX - DEAD_ZONE_Y_MIN),
                       color=(0, 255, 0), thickness=2)

    # 获取所有检测结果
    detections = net.predict([img], callback=fomo_post_process)

    # 先标记所有类别为未检测状态
    detected_labels = set()

    # 处理每个检测到的类别
    for label_index, detection_list in enumerate(detections):
        if label_index == 0:  # 跳过背景类别
            continue

        # 过滤出位于检测区域内的目标
        zone_detections = []
        for det in detection_list:
            x, y, w, h, score = det
            center_x = math.floor(x + (w / 2))
            center_y = math.floor(y + (h / 2))
            if is_inside_detection_zone(center_x, center_y):
                zone_detections.append(det)

        if not zone_detections:  # 如果区域内没有检测到目标
            reset_tracking_data(label_index)
            continue

        detected_labels.add(label_index)  # 标记为已检测

       # print(f"********** {labels[label_index]} **********")
        # 选择区域内置信度最高的目标
        best_detection = max(zone_detections, key=lambda d: d[4])
        x, y, w, h, score = best_detection
        center_x = math.floor(x + (w / 2))
        center_y = math.floor(y + (h / 2))
        #print(f"x {center_x}\ty {center_y}\tscore {score}")
        img.draw_circle((center_x, center_y, 12), color=colors[label_index % len(colors)])

        # 更新检测历史，保持最近2次记录
        positions = tracking_data[label_index]['positions']
        positions.append((center_x, center_y))
        if len(positions) > 2:
            positions.pop(0)
        tracking_data[label_index]['positions'] = positions

        # 检查是否满足发送条件
        if len(positions) == 2 and not tracking_data[label_index]['notified']:
            if check_position_consistency(positions, 15):
                # 通过串口发送动物名称
                animal_name = labels[label_index]
                # 替换原来的串口输出部分
                uart.write(bytes([0xAA]))       # 发送起始字节AA
                uart.write(bytes([0x55]))       # 发送分隔字节55
                uart.write(str(label_index).encode())  # 将标签编号转为字符串再编码为字节
                uart.write(bytes([0x5D]))       # 发送结束字节5D
                print(f"Sent to UART1: Detected {label_index}")
                tracking_data[label_index]['notified'] = True  # 标记为已发送

    # 对本轮未检测到的类别，重置其跟踪数据
    for label_index in tracking_data:
        if label_index != 0 and label_index not in detected_labels:
            reset_tracking_data(label_index)

   # print(clock.fps(), "fps", end="\n\n")


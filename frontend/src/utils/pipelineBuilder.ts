import { NodeInstance, CvedixNodeType } from '@/models';
import { BackendPipelineRequest, BackendNodeConfig } from '@/api/pipeline.api';

/**
 * Build complete pipeline JSON for backend
 * Supports two pipeline types:
 * - Object Detection: Source -> YOLO -> Tracker -> Crossline -> Crossline OSD -> RTMP
 * - Face Detection: Source -> Face Detector -> Face OSD -> RTMP
 */
export const buildBackendPipeline = (nodes: NodeInstance[]): BackendPipelineRequest => {
  const backendNodes: BackendNodeConfig[] = [];

  // Find all nodes
  const sourceNode = nodes.find((n) => n.type === CvedixNodeType.FILE_SOURCE);
  const yoloNode = nodes.find((n) => n.type === CvedixNodeType.YOLO_DETECTOR);
  const trackerNode = nodes.find((n) => n.type === CvedixNodeType.DSORT_TRACKER);
  const crosslineNode = nodes.find((n) => n.type === CvedixNodeType.BA_CROSSLINE);
  const crosslineOsdNode = nodes.find((n) => n.type === CvedixNodeType.BA_CROSSLINE_OSD);
  const faceDetectorNode = nodes.find((n) => n.type === CvedixNodeType.YUNET_FACE_DETECTOR);
  const faceOsdNode = nodes.find((n) => n.type === CvedixNodeType.FACE_OSD);
  const destinationNode = nodes.find((n) => n.type === CvedixNodeType.RTMP_DESTINATION);

  // Build pipeline in correct order

  // 1. File source node
  if (sourceNode) {
    backendNodes.push({
      type: sourceNode.type,
      config: {
        node_name: sourceNode.data.config.node_name || 'file_src',
        channel_index: sourceNode.data.config.channel_index ?? 0,
        video_name: sourceNode.data.config.video_name || '',
        resize_ratio: sourceNode.data.config.resize_ratio ?? 1.0,
        cycle: sourceNode.data.config.cycle ?? true,
        gst_decoder_name: sourceNode.data.config.gst_decoder_name || 'avdec_h264',
        skip_interval: sourceNode.data.config.skip_interval ?? 1,
      },
    });
  }

  // 2. YOLO Detector node (Object Detection Pipeline)
  if (yoloNode) {
    backendNodes.push({
      type: yoloNode.type,
      config: {
        node_name: yoloNode.data.config.node_name || 'yolo_detector',
        input_width: yoloNode.data.config.input_width ?? 416,
        input_height: yoloNode.data.config.input_height ?? 416,
        batch_size: yoloNode.data.config.batch_size ?? 1,
        class_id_offset: yoloNode.data.config.class_id_offset ?? 0,
        score_threshold: yoloNode.data.config.score_threshold ?? 0.5,
        confidence_threshold: yoloNode.data.config.confidence_threshold ?? 0.5,
        nms_threshold: yoloNode.data.config.nms_threshold ?? 0.5,
        scale: yoloNode.data.config.scale ?? 0.0039215686,
        mean_r: yoloNode.data.config.mean_r ?? 0.0,
        mean_g: yoloNode.data.config.mean_g ?? 0.0,
        mean_b: yoloNode.data.config.mean_b ?? 0.0,
        std_r: yoloNode.data.config.std_r ?? 1.0,
        std_g: yoloNode.data.config.std_g ?? 1.0,
        std_b: yoloNode.data.config.std_b ?? 1.0,
        swap_rb: yoloNode.data.config.swap_rb ?? true,
      },
    });
  }

  // 3. DeepSORT Tracker node
  if (trackerNode) {
    backendNodes.push({
      type: trackerNode.type,
      config: {
        node_name: trackerNode.data.config.node_name || 'dsort_tracker',
      },
    });
  }

  // 4. BA Crossline node
  if (crosslineNode) {
    backendNodes.push({
      type: crosslineNode.type,
      config: {
        node_name: crosslineNode.data.config.node_name || 'ba_crossline',
        need_record_image: crosslineNode.data.config.need_record_image ?? true,
        need_record_video: crosslineNode.data.config.need_record_video ?? false,
      },
    });
  }

  // 5. BA Crossline OSD node
  if (crosslineOsdNode) {
    backendNodes.push({
      type: crosslineOsdNode.type,
      config: {
        node_name: crosslineOsdNode.data.config.node_name || 'osd',
        font: crosslineOsdNode.data.config.font || '/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf',
      },
    });
  }

  // 6. Yunet Face Detector node (Face Detection Pipeline)
  if (faceDetectorNode) {
    backendNodes.push({
      type: faceDetectorNode.type,
      config: {
        node_name: faceDetectorNode.data.config.node_name || 'face_detector',
      },
    });
  }

  // 7. Face OSD node
  if (faceOsdNode) {
    backendNodes.push({
      type: faceOsdNode.type,
      config: {
        node_name: faceOsdNode.data.config.node_name || 'osd',
      },
    });
  }

  // 8. RTMP destination node (always last)
  if (destinationNode) {
    backendNodes.push({
      type: destinationNode.type,
      config: {
        node_name: destinationNode.data.config.node_name || 'rtmp_node',
        channel_index: destinationNode.data.config.channel_index ?? 0,
        rtmp_url: destinationNode.data.config.rtmp_url || 'rtmp://anhoidong.datacenter.cvedix.com:1935/live/stream',
        resolution_width: destinationNode.data.config.resolution_width ?? 360,
        resolution_height: destinationNode.data.config.resolution_height ?? 240,
        bitrate: destinationNode.data.config.bitrate ?? 600,
        osd: destinationNode.data.config.osd ?? true,
        gst_encoder_name: destinationNode.data.config.gst_encoder_name || 'x264enc',
      },
    });
  }

  return {
    nodes: backendNodes,
  };
};

/**
 * Validate that pipeline has required nodes
 */
export const validatePipeline = (nodes: NodeInstance[]): { valid: boolean; error?: string } => {
  const hasSource = nodes.some((n) => n.type === CvedixNodeType.FILE_SOURCE);
  const hasDestination = nodes.some((n) => n.type === CvedixNodeType.RTMP_DESTINATION);

  // Check for object detection pipeline nodes
  const hasYolo = nodes.some((n) => n.type === CvedixNodeType.YOLO_DETECTOR);
  const hasTracker = nodes.some((n) => n.type === CvedixNodeType.DSORT_TRACKER);
  const hasCrossline = nodes.some((n) => n.type === CvedixNodeType.BA_CROSSLINE);
  const hasCrosslineOsd = nodes.some((n) => n.type === CvedixNodeType.BA_CROSSLINE_OSD);

  // Check for face detection pipeline nodes
  const hasFaceDetector = nodes.some((n) => n.type === CvedixNodeType.YUNET_FACE_DETECTOR);
  const hasFaceOsd = nodes.some((n) => n.type === CvedixNodeType.FACE_OSD);

  if (!hasSource) {
    return { valid: false, error: 'Pipeline must have a Video Source node' };
  }

  if (!hasDestination) {
    return { valid: false, error: 'Pipeline must have an RTMP Stream node' };
  }

  // Check source node has video selected
  const sourceNode = nodes.find((n) => n.type === CvedixNodeType.FILE_SOURCE);
  if (sourceNode && !sourceNode.data.config.video_name) {
    return { valid: false, error: 'Video Source node must have a video selected' };
  }

  // Check that at least one processing pipeline is present
  const hasObjectDetectionPipeline = hasYolo || hasTracker || hasCrossline || hasCrosslineOsd;
  const hasFaceDetectionPipeline = hasFaceDetector || hasFaceOsd;

  if (!hasObjectDetectionPipeline && !hasFaceDetectionPipeline) {
    console.warn('Pipeline has no processing nodes - only source and destination');
  }

  return { valid: true };
};

/**
 * Get the correct pipeline order for nodes
 * Order: Source -> Object Detection nodes -> Face Detection nodes -> RTMP
 */
const getPipelineOrder = (nodeType: string): number => {
  const order: Record<string, number> = {
    [CvedixNodeType.FILE_SOURCE]: 0,
    // Object Detection Pipeline (order 1-4)
    [CvedixNodeType.YOLO_DETECTOR]: 1,
    [CvedixNodeType.DSORT_TRACKER]: 2,
    [CvedixNodeType.BA_CROSSLINE]: 3,
    [CvedixNodeType.BA_CROSSLINE_OSD]: 4,
    // Face Detection Pipeline (order 5-6)
    [CvedixNodeType.YUNET_FACE_DETECTOR]: 5,
    [CvedixNodeType.FACE_OSD]: 6,
    // Output (always last)
    [CvedixNodeType.RTMP_DESTINATION]: 100,
  };
  return order[nodeType] ?? 99;
};

/**
 * Auto-generate connections for visual representation
 * Returns array of connection objects for ReactFlow
 * Connections are made in pipeline order: Source -> YOLO -> Tracker -> Crossline -> OSD -> RTMP
 */
export const generateAutoConnections = (nodes: NodeInstance[]) => {
  const connections = [];

  // Sort nodes by pipeline order
  const sortedNodes = [...nodes].sort(
    (a, b) => getPipelineOrder(a.type) - getPipelineOrder(b.type)
  );

  // Create connections between consecutive nodes
  for (let i = 0; i < sortedNodes.length - 1; i++) {
    const sourceNode = sortedNodes[i];
    const targetNode = sortedNodes[i + 1];

    connections.push({
      id: `${sourceNode.id}-${targetNode.id}`,
      source: sourceNode.id,
      target: targetNode.id,
      animated: true,
    });
  }

  return connections;
};

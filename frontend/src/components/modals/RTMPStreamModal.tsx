import React, { useEffect, useRef, useState, useCallback } from 'react';
import {
  Dialog,
  DialogContent,
  DialogDescription,
  DialogHeader,
  DialogTitle,
} from '@/components/ui/dialog';
import { Button } from '@/components/ui/button';
import videojs from 'video.js';
import 'video.js/dist/video-js.css';
import type Player from 'video.js/dist/types/player';

interface RTMPStreamModalProps {
  open: boolean;
  rtmpUrl: string;
  onClose: () => void;
  onStop: () => void;
}

export const RTMPStreamModal: React.FC<RTMPStreamModalProps> = ({
  open,
  rtmpUrl,
  onClose,
  onStop,
}) => {
  const playerRef = useRef<Player | null>(null);
  const [videoElement, setVideoElement] = useState<HTMLVideoElement | null>(null);
  const [isPlaying, setIsPlaying] = useState(false);

  // Callback ref to capture video element when it mounts
  const videoRefCallback = useCallback((node: HTMLVideoElement | null) => {
    setVideoElement(node);
  }, []);

  // Build HLS URL from RTMP URL
  const getHlsUrl = () => {
    const hlsBaseUrl = import.meta.env.VITE_HLS_BASE_URL || 'https://spatial-transcripts-ddr-compound.trycloudflare.com';
    const rtmpUrl_parsed = new URL(rtmpUrl);
    const streamPath = rtmpUrl_parsed.pathname;
    return `${hlsBaseUrl}${streamPath}/hls.m3u8`;
  };

  // Initialize player when video element is available
  useEffect(() => {
    if (!open || !videoElement) {
      return;
    }

    // Don't reinitialize if player already exists
    if (playerRef.current) {
      return;
    }

    const hlsUrl = getHlsUrl();
    console.log('Initializing player with HLS URL:', hlsUrl);

    // Create player
    const player = videojs(videoElement, {
      controls: true,
      autoplay: true,
      muted: true,
      preload: 'auto',
      liveui: true,
      html5: {
        vhs: {
          overrideNative: true,
          enableLowInitialPlaylist: true,
        },
        nativeVideoTracks: false,
        nativeAudioTracks: false,
        nativeTextTracks: false,
      },
    });

    player.on('playing', () => {
      setIsPlaying(true);
    });

    player.on('pause', () => {
      setIsPlaying(false);
    });

    // Load and play
    player.src({
      src: hlsUrl,
      type: 'application/x-mpegURL',
    });

    playerRef.current = player;

    return () => {
      if (playerRef.current) {
        playerRef.current.dispose();
        playerRef.current = null;
      }
    };
  }, [open, videoElement]);

  // Reset when modal closes
  useEffect(() => {
    if (!open) {
      setIsPlaying(false);
    }
  }, [open]);

  const handleStop = () => {
    if (playerRef.current) {
      playerRef.current.pause();
    }
    onStop();
  };

  return (
    <Dialog open={open} onOpenChange={onClose}>
      <DialogContent className="max-w-4xl">
        <DialogHeader>
          <DialogTitle>Live RTMP Stream</DialogTitle>
          <DialogDescription>
            Streaming to: {rtmpUrl}
          </DialogDescription>
        </DialogHeader>

        <div className="space-y-4">
          {/* Video Player */}
          <div className="bg-black rounded-lg overflow-hidden" data-vjs-player>
            <video
              ref={videoRefCallback}
              className="video-js vjs-big-play-centered"
              style={{ width: '100%', height: '500px' }}
              playsInline
              muted
            />
          </div>

          {/* Stream Info */}
          <div className="bg-gray-50 p-4 rounded-lg text-sm">
            <div className="grid grid-cols-2 gap-2">
              <div>
                <span className="font-medium text-gray-700">Status:</span>
                {isPlaying ? (
                  <span className="ml-2 text-green-600">● Live</span>
                ) : (
                  <span className="ml-2 text-yellow-600">● Connecting...</span>
                )}
              </div>
              <div>
                <span className="font-medium text-gray-700">URL:</span>
                <span className="ml-2 text-gray-600 font-mono text-xs truncate block">
                  {rtmpUrl}
                </span>
              </div>
            </div>
          </div>

          {/* Controls */}
          <div className="flex justify-end gap-2">
            <Button variant="outline" onClick={onClose}>
              Minimize
            </Button>
            <Button variant="destructive" onClick={handleStop}>
              Stop Pipeline
            </Button>
          </div>
        </div>
      </DialogContent>
    </Dialog>
  );
};

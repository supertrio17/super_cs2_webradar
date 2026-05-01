import { useRef } from "react";
import { getRadarPosition, calculatePositionWithScale } from "../utilities/utilities";

const GrenadeEffects = ({ grenadeData, type, mapData, averageLatency, radarImage }) => {

    let radarScale = 1; try { let scale = radarImage.style.scale; if (scale) radarScale = scale;} catch {}

    const smokeSize = 3 * radarScale;
    const fireSize = 1.3 * radarScale;
    const radarPosition = getRadarPosition(mapData, { x: grenadeData.m_x, y: grenadeData.m_y });

    const grenRef = useRef();
    const grenBounding = (grenRef.current &&
      grenRef.current.getBoundingClientRect()) || { width: 0, height: 0 };

    const scaledPos = calculatePositionWithScale(radarImage, radarPosition);
    const radarImageTranslation = {
      x: (scaledPos[0] - grenBounding.width * 0.5),
      y: (scaledPos[1] - grenBounding.height * 0.5),
    };

  if (type == "smoke") {
    const smokeDuration = grenadeData.m_duration || 21.5;
    const smokeRatio = Math.max(0, Math.min(1, (grenadeData.m_timeleft || 0) / smokeDuration));

    return (
        <div
        ref={grenRef}
        key={grenadeData.m_idx}
        className={`absolute rounded-[100%] left-0 top-0`}
        style={{
          width: `${smokeSize}vw`,
          height: `${smokeSize}vw`,
          transform: `translate(${radarImageTranslation.x}px, ${radarImageTranslation.y}px)`,
          transition: `transform ${averageLatency}ms linear`,
          opacity: `0.7`,
          zIndex: `2`,
        }}
        >

        <div
          className={`absolute origin-center rounded-[100%] bg-gray-500`}
          style={{
            width: `100%`,
            height: `100%`,
            filter: `blur(4px)`,
          }}
        />

        <label className="absolute left-1/2 top-1/2 -translate-x-1/2 -translate-y-1/2 text-white text-xs font-bold rounded-lg border border-white/20 bg-black/55 px-2 py-1 backdrop-blur-sm">
          {grenadeData.m_timeleft.toFixed(1)}s
          <span className="block mt-1 h-[2px] w-full rounded bg-white/20 overflow-hidden">
            <span
              className="block h-full rounded bg-slate-200"
              style={{ width: `${smokeRatio * 100}%` }}
            />
          </span>
        </label>

        </div>
    );
  } else if (type == "molo") {
    return (
      <div
        ref={grenRef}
        className={`absolute w-full h-full origin-center rounded-[100%] bg-orange-500`}
        style={{
          width: `${fireSize}vw`,
          height: `${fireSize}vw`,
          transform: `translate(${radarImageTranslation.x}px, ${radarImageTranslation.y}px)`,
          transition: `transform ${averageLatency}ms linear`,
          filter: `blur(2px)`,
          zIndex: `1`,
        }}
      />
    );
  }
};

export default GrenadeEffects;

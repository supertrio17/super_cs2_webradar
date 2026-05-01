/* eslint-disable react/prop-types */
import { useRef } from "react";
import GrenadeEffects from "./grenadeeffects.jsx";
import { getRadarPosition, calculatePositionWithScale } from "../utilities/utilities";
import MaskedIcon from "./maskedicon.jsx";

const Grenade = ({ grenadeData, mapData, settings, averageLatency, radarImage, type, trailPoints = [], tempPlayer }) => {
  const firePositions = grenadeData.m_firePositions || [];

  const radarPosition = getRadarPosition(mapData, { x: grenadeData.m_x, y: grenadeData.m_y });

  const grenRef = useRef();
  const grenBounding = (grenRef.current &&
    grenRef.current.getBoundingClientRect()) || { width: 0, height: 0 };
  const scaledPos = calculatePositionWithScale(radarImage, radarPosition);
  const radarImageTranslation = {
    x: (scaledPos[0] - grenBounding.width * 0.5),
    y: (scaledPos[1] - grenBounding.height * 0.5),
  };

  const grenadeDuration = grenadeData.m_duration || (grenadeData.m_type == "smoke" ? 21.5 : 7);
  const grenadeDurationRatio = Math.max(0, Math.min(1, (grenadeData.m_timeleft || 0) / grenadeDuration));

  const ownerEntryIdx = Number.isFinite(grenadeData?.m_owner_entry_idx) ? grenadeData.m_owner_entry_idx : -1;
  const selectedPawnEntryIdx = Number.isFinite(tempPlayer?.m_pawn_entry_idx) ? tempPlayer.m_pawn_entry_idx : -1;
  const isThrownBySelectedPlayer = ownerEntryIdx >= 0 && selectedPawnEntryIdx >= 0 && ownerEntryIdx === selectedPawnEntryIdx;

  const trailSegments = [];
  if (type == "thrown" && trailPoints.length > 1 && radarImage) {
    for (let idx = 1; idx < trailPoints.length; idx++) {
      const start = trailPoints[idx - 1];
      const end = trailPoints[idx];

      const startPoint = calculatePositionWithScale(radarImage, getRadarPosition(mapData, { x: start.m_x, y: start.m_y }));
      const endPoint = calculatePositionWithScale(radarImage, getRadarPosition(mapData, { x: end.m_x, y: end.m_y }));

      const dx = endPoint[0] - startPoint[0];
      const dy = endPoint[1] - startPoint[1];
      const segmentLength = Math.hypot(dx, dy);
      if (!segmentLength || segmentLength < 1)
        continue;

      const segmentOpacity = idx / trailPoints.length;
      const segmentRotation = Math.atan2(dy, dx) * (180 / Math.PI);

      trailSegments.push(
        <div
          key={`${grenadeData.m_idx}-trail-${idx}`}
          className="absolute left-0 top-0 pointer-events-none"
          style={{
            transform: `translate(${startPoint[0]}px, ${startPoint[1]}px) rotate(${segmentRotation}deg)`,
            transformOrigin: `0 50%`,
            width: `${segmentLength}px`,
            height: `2px`,
            borderRadius: `999px`,
            background: `linear-gradient(90deg, rgba(255, 255, 255, ${0.55 * segmentOpacity}), rgba(255, 255, 255, 0.02))`,
          }}
        />
      );
    }
  }

  return (
    <>
      {trailSegments}

      {(type == "landed") ? (
      <>
        {(grenadeData.m_type == "molo") ? (
          <><div
            key={grenadeData.m_idx}
            className={`absolute rounded-[100%] left-0 top-0`}
            style={{
              opacity: `0.4`,
            }}
          >

            {firePositions[0] != null && firePositions.map((firePosition, index) => (
              <GrenadeEffects
                key={index}
                grenadeData={{ m_x: firePosition[0], m_y: firePosition[1] }}
                type="molo"
                mapData={mapData}
                settings={settings}
                averageLatency={averageLatency}
                radarImage={radarImage} />
            ))}

          </div><div
            className={`absolute rounded-[100%] left-0 top-0`}
            style={{
              transform: `translate(${radarImageTranslation.x}px, ${radarImageTranslation.y}px)`,
              transition: `transform ${averageLatency}ms linear`,
            }}
          >

              <label className="absolute w-full text-center text-white text-xs font-bold rounded-lg border border-white/20 bg-black/55 px-2 py-1 backdrop-blur-sm">
                {`${grenadeData.m_timeleft.toFixed(1)}s${isThrownBySelectedPlayer ? " • YOU" : ""}`}
                <span className="block mt-1 h-[2px] w-full rounded bg-white/20 overflow-hidden">
                  <span
                    className="block h-full rounded bg-orange-300"
                    style={{ width: `${grenadeDurationRatio * 100}%` }}
                  />
                </span>
              </label>

            </div></>
        ) : (
          <GrenadeEffects
            grenadeData={grenadeData}
            type="smoke"
            mapData={mapData}
            settings={settings}
            averageLatency={averageLatency}
            radarImage={radarImage}
            isOwnedBySelectedPlayer={isThrownBySelectedPlayer}
          />
        )}

      </>
      ) : (
        <div
        ref={grenRef}
        className={`absolute left-0 top-0`}
        style={{
          transform: `translate(${radarImageTranslation.x}px, ${radarImageTranslation.y}px)`,
          transition: `transform ${averageLatency}ms linear`,
        }}
        >

          <MaskedIcon
            path={`./assets/icons/${grenadeData.m_type}.svg`}
            height={`${settings.thrownGrenadeSize}vw`}
            color={`${isThrownBySelectedPlayer ? "#5fffe0" : settings.thrownGrenadeColor}`}
          />

          {isThrownBySelectedPlayer && (
            <label className="absolute left-1/2 top-full mt-1 -translate-x-1/2 whitespace-nowrap text-[10px] font-bold text-cyan-200">
              YOU
            </label>
          )}

        </div>
      )}
    </>
  );

};

export default Grenade;

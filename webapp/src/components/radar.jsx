import { useEffect, useRef, useState } from "react";
import Draggable from "./Draggable"
import Player from "./player";
import Bomb from "./bomb";
import Grenade from "./grenade";
import DroppedWeapon from "./droppedweapons";

const GRENADE_TRAIL_MAX_AGE = 2200;
const GRENADE_TRAIL_MAX_POINTS = 24;

const Radar = ({
  playerArray,
  radarImage,
  mapData,
  localTeam,
  averageLatency,
  bombData,
  settings,
  grenadeData,
  droppedWeaponsData,
  tempPlayer,
  radarZoom,
  radarScale,
}) => {
  const radarImageRef = useRef();
  const radarContentRef = useRef();
  const grenadeTrailRef = useRef({});
  const [grenadeTrailData, setGrenadeTrailData] = useState({});

  useEffect(() => {
    const now = Date.now();
    const currentThrown = grenadeData?.thrown || [];
    const activeIdx = new Set();

    currentThrown.forEach((grenade) => {
      const key = `${grenade.m_idx}`;
      activeIdx.add(key);

      const existingTrail = grenadeTrailRef.current[key] || [];
      const lastPoint = existingTrail[existingTrail.length - 1];
      const movedEnough = !lastPoint || Math.hypot((grenade.m_x - lastPoint.m_x), (grenade.m_y - lastPoint.m_y)) > 10;

      if (movedEnough) {
        existingTrail.push({ m_x: grenade.m_x, m_y: grenade.m_y, m_created_at: now });
      }

      grenadeTrailRef.current[key] = existingTrail
        .filter((point) => now - point.m_created_at <= GRENADE_TRAIL_MAX_AGE)
        .slice(-GRENADE_TRAIL_MAX_POINTS);
    });

    Object.keys(grenadeTrailRef.current).forEach((idx) => {
      const filteredTrail = (grenadeTrailRef.current[idx] || []).filter((point) => now - point.m_created_at <= GRENADE_TRAIL_MAX_AGE);
      if (!activeIdx.has(idx) && filteredTrail.length === 0) {
        delete grenadeTrailRef.current[idx];
        return;
      }

      grenadeTrailRef.current[idx] = filteredTrail;
    });

    setGrenadeTrailData({ ...grenadeTrailRef.current });
  }, [grenadeData]);

  return (
    <div id="radar" className={`relative overflow-hidden origin-center`} ref={radarContentRef}>

      <Draggable
      imgref={radarImageRef.current}
      avrPing={averageLatency}
      radarContentRef={radarContentRef.current}
      >
        <img onWheelCapture={radarZoom} ref={radarImageRef} className={`w-full h-auto rounded-[20px] ${bombData && bombData.m_blow_time <= 10 && tempPlayer && !tempPlayer.m_is_dead && tempPlayer.m_bomb_damage>=tempPlayer.m_health?`alertAnim`:``}`} src={radarImage} draggable={false} style={{scale: `${radarScale}`, transition: "scale 150ms linear", filter: `contrast(${200-settings.mapBrightness||100}%) brightness(${settings.mapBrightness||100}%)`}}/>
      </Draggable>

      {playerArray && playerArray!=null && playerArray.map((player) => (
        <Player
          key={player.m_idx}
          playerData={player}
          mapData={mapData}
          radarImage={radarImageRef.current}
          radarScale={radarScale}
          localTeam={localTeam}
          averageLatency={averageLatency}
          settings={settings}
          tempPlayer={tempPlayer}
        />
      ))}

      {bombData && (
        <Bomb
          bombData={bombData}
          mapData={mapData}
          radarImage={radarImageRef.current}
          localTeam={localTeam}
          averageLatency={averageLatency}
          settings={settings}
        />
      )}

      {grenadeData && grenadeData["landed"]!=null && settings.showGrenades && grenadeData["landed"].map((grenade) => (
        <Grenade
          key={grenade.m_idx}
          grenadeData={grenade}
          mapData={mapData}
          settings={settings}
          averageLatency={averageLatency}
          radarImage={radarImageRef.current}
          type={"landed"}
        />
      ))}

      {grenadeData && grenadeData["thrown"]!=null && settings.showGrenades && grenadeData["thrown"].map((grenade) => (
        <Grenade
          key={grenade.m_idx}
          grenadeData={grenade}
          mapData={mapData}
          settings={settings}
          averageLatency={averageLatency}
          radarImage={radarImageRef.current}
          type={"thrown"}
          trailPoints={grenadeTrailData[`${grenade.m_idx}`] || []}
        />
      ))}

      {settings.showDroppedWeapons && droppedWeaponsData && droppedWeaponsData!=null && droppedWeaponsData.map((droppedWeapon) => (
        <DroppedWeapon
          key={droppedWeapon.m_idx}
          droppedWeaponData={droppedWeapon}
          mapData={mapData}
          settings={settings}
          averageLatency={averageLatency}
          radarImage={radarImageRef.current}
        />
      ))}

    </div>
  );
};

export default Radar;

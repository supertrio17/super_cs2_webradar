/* eslint-disable react/prop-types */
import { useRef } from "react";
import { getRadarPosition, teamEnum, calculatePositionWithScale } from "../utilities/utilities";

const Bomb = ({ bombData, mapData, radarImage, localTeam, averageLatency, settings, tempPlayer }) => {
  const radarPosition = getRadarPosition(mapData, bombData);

  const bombRef = useRef();
  const bombBounding = (bombRef.current &&
    bombRef.current.getBoundingClientRect()) || { width: 0, height: 0 };

  const scaledPos = calculatePositionWithScale(radarImage, radarPosition);
  const radarImageTranslation = {
    x: (scaledPos[0] - bombBounding.width * 0.5),
    y: (scaledPos[1] - bombBounding.height * 0.5),
  };

  const ownerEntryIdx = Number.isFinite(bombData?.owner_entry_idx) ? bombData.owner_entry_idx : -1;
  const selectedPawnEntryIdx = Number.isFinite(tempPlayer?.m_pawn_entry_idx) ? tempPlayer.m_pawn_entry_idx : -1;

  const ownerIsValid = ownerEntryIdx >= 0 && ownerEntryIdx < 0x7fff;
  const carriedBySelectedPlayer = ownerIsValid && selectedPawnEntryIdx >= 0 && ownerEntryIdx === selectedPawnEntryIdx;
  const hideCarriedBomb = settings.showOnlyEnemies && localTeam == teamEnum.terrorist && ownerIsValid && !carriedBySelectedPlayer;

  const baseSize = 1.5;
  const scaledSize = baseSize * settings.bombSize;

  return (
    <div
      className={`absolute origin-center rounded-[100%] left-0 top-0`}
      ref={bombRef}
      style={{
        width: `${scaledSize}vw`,
        height: `${scaledSize}vw`,
        transform: `translate(${radarImageTranslation.x}px, ${radarImageTranslation.y}px)`,
        transition: `transform ${averageLatency}ms linear`,
        backgroundColor: `${
          (bombData.m_is_defused && `#50904c`) ||
          (localTeam == teamEnum.counterTerrorist && `#6492b4`) ||
          `#c90b0b`
        }`,
        WebkitMask: `url('./assets/icons/c4_sml.png') no-repeat center / contain`,
        opacity: `${hideCarriedBomb ? 0 : 1}`,
        zIndex: `1`,
      }}
    />
  );
};

export default Bomb;
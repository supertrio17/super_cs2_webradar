/* eslint-disable react/prop-types */
/* eslint-disable react-refresh/only-export-components */
import SettingsButton from "./SettingsButton";

let latencyData = {
  averageCount: 0,
  averageSum: 0,
  averageTime: 0,
  lastTime: 0,
};

export const resetLatency = () => {
  latencyData = {
    averageCount: 0,
    averageSum: 0,
    averageTime: 0,
    lastTime: performance.now(),
  };
};

export const getLatency = () => {
  const currentTime = performance.now();

  if (!latencyData.lastTime) {
    latencyData.lastTime = currentTime;
    return latencyData.averageTime;
  }

  const diffInMs = currentTime - latencyData.lastTime;
  latencyData.lastTime = currentTime;

  if (!Number.isFinite(diffInMs) || diffInMs <= 0) {
    return latencyData.averageTime;
  }

  if (latencyData.averageTime === 0) {
    latencyData.averageTime = diffInMs;
  }

  latencyData.averageCount++;
  latencyData.averageSum += diffInMs;

  if (latencyData.averageCount >= 5) {
    latencyData.averageTime = latencyData.averageSum / latencyData.averageCount;

    latencyData.averageCount = 0;
    latencyData.averageSum = 0;
  }

  return latencyData.averageTime;
};

export const Latency = ({ value, settings, setSettings, translation, languages }) => {
  return (
    <div className={`flex gap-2 absolute text-[normal] right-2.5 top-2.5`}>
      <div className={'flex gap-1'}>
        <img className={`w-[1.3rem]`} src={`./assets/icons/gauge.svg`} />
        <span>{value.toFixed(0)}ms</span>
      </div>

      <SettingsButton settings={settings} onSettingsChange={setSettings} translation={translation} languageOptions={languages}/>
    </div>
  );
};

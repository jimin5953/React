import { useSyncExternalStore } from "react";
function subscribe(callback) {
  window.addEventListener("resize", callback);
  return () => {
    window.removeEventListener("resize", callback);
  };
}
const useInnerHeight = () => {
  const innerHeight = useSyncExternalStore(subscribe, () => window.innerHeight);
  return innerHeight;
};
export default useInnerHeight;

#pragma once
// Keep the pair centered. Only eyelids/expressions animate; no wandering or shaking.
template<typename Eyes>
void configureCenteredFace(Eyes &value) {
  value.setWidth(34,34); value.setHeight(38,38);
  value.setBorderradius(10,10); value.setSpacebetween(20);
  value.setCuriosity(false); value.setIdleMode(false);
  value.setHFlicker(false); value.setVFlicker(false);
  value.eyeLwidthCurrent=value.eyeRwidthCurrent=34;
  value.spaceBetweenCurrent=20;
  value.setPosition(0);
  value.eyeLx=value.eyeLxNext;
  value.eyeLy=value.eyeRy=value.eyeLyNext;
  value.eyeRx=value.eyeRxNext=value.eyeLxNext+54;
  value.eyeRyNext=value.eyeLyNext;
}

mit(float BPM, float IE_ratio, float pressure_mask, float pressure_expiration, float TidVol, float pressure_diff){
  p_mask = map(pressure_mask, 5.00, 20.00, 0, 255);
  p_expiration = map(pressure_mask, 5.00, 20.00, 0, 255);
  p_diff = map(pressure_mask, 5.00, 20.00, 0, 255);
  String to_send_p_mask = ad + id_1 + "," + ch + "," + int(p_mask);
  print_screen(to_send_p_mask);
  String to_send_p_diff = ad + id_2 + "," + ch + "," + int(p_diff);
  print_screen(to_send_p_diff);
  String to_send_p_exp = ad + id_1 + "," + ch + "," + int(p_expiration);
  print_screen(to_send_p_exp);
  dtostrf(BPM, 6, 2, buffer);
  t0.setText(buffer);
  dtostrf(IE_ratio,6,2,buffer_2);
  t1.setText(buffer_2);
  dtostrf(TidVol,6,2,buffer_2);
  t1.setText(buffer_3);
}


void print_screen(String to_send){
  Serial.print(to_send);
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
}

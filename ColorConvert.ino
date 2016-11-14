void HSV2RGB(int h_, int s_, int l_, int rgb[]) {

    float h=(float)h_/255.0;
    float s=(float)s_/255.0;
    float l=(float)l_/255.0;
  
    double r, g, b;

    if (s == 0) {
        r = g = b = l; // achromatic
    } else {
        double q = l < 0.5 ? l * (1 + s) : l + s - l * s;
        double p = 2 * l - q;
        r = hue2rgb(p, q, h + 1/3.0);
        g = hue2rgb(p, q, h);
        b = hue2rgb(p, q, h - 1/3.0);
    }

    rgb[0] = r * 255;
    rgb[1] = g * 255;
    rgb[2] = b * 255;

//    Serial.print(h_);
//    Serial.print(" ");
//    Serial.print(s_);
//    Serial.print(" ");
//    Serial.print(l_);
//    Serial.print(" -> ");
//    
//    for(int i=0;i<3;++i){
//      Serial.print(rgb[i]);
//      Serial.print(" ");
//    }
//    Serial.println();
}

double hue2rgb(double p, double q, double t) {
    if(t < 0) t += 1;
    if(t > 1) t -= 1;
    if(t < 1/6.0) return p + (q - p) * 6 * t;
    if(t < 1/2.0) return q;
    if(t < 2/3.0) return p + (q - p) * (2/3.0 - t) * 6;
    return p;
}

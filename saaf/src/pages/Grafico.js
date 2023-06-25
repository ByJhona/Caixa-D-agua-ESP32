import { Chart } from "react-google-charts";
import {database, app} from '../services/firebase'
import { getDatabase, ref, onValue} from "firebase/database";
import React, { useEffect, useState } from "react";


export const dataDefault = [
  ["Tempo em segundos", "Nível de água"],
  ['00-00-00 00:00:00', 0],
  
];

export const options = {
  title: "Nivel de Agua no Reservatorio",
  hAxis: { title: "Tempo(Dias)", titleTextStyle: { color: "#333" }, viewWindowMode: 'explicit',
  viewWindow: {
    min: 0
  } },
  vAxis: { title: "Litros(L)",minValue: 0 , viewWindowMode: 'explicit',
  viewWindow: {
    max: 100,
    min: 0
  }},
  chartArea: { width: "70%", height: "40%" },
  
};

export default function Grafico() {
  
  const [nivelAgua, setNivelAgua] = useState([]);

  useEffect(() => {
    console.log("NAOOO")
    
    const nivelAguaRef = ref(getDatabase(), 'nivel_agua/');
    onValue(nivelAguaRef, (snapshot) => {
      const snap = snapshot.val();
      if(!snapshot.exists()) {
        setNivelAgua(dataDefault);
        return;
      };
      setNivelAgua([["Tempo em segundos", "Nível de água"]]);

      
      for (const [key, value] of Object.entries(snap)) {
        
        setNivelAgua((dado) => [...dado, [key, value]]);
        

      }
      console.log(nivelAgua)
      
      
    });
  }, []);
    return (
      <>
        
    {console.log(dataDefault)}
    {console.log(nivelAgua)}
    <Chart
      chartType="AreaChart"
      width="100%"
      
      height="100%"
      data={nivelAgua}
      options={options}
    />
  
      </>

    )
  }
  
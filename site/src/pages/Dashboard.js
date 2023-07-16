import NivelAgua from "./NivelAgua"
import NivelTurbidez from "./NivelTurbidez"
import styles from '../styles/Dashboard.module.css'
import { useEffect, useState } from "react"
import { getDatabase, ref, onValue, update } from "firebase/database";


export default function Dashboard() {

  const [estadoRele, setEstadoRele] = useState(false);

  useEffect(() => {
    const estadoReleRef = ref(getDatabase());
    update(estadoReleRef, { estado_rele: estadoRele })
    console.log("TESTE")

  }, [estadoRele]);

  useEffect(() => {
    const estadoReleRef = ref(getDatabase(), '/estado_rele/');
    onValue(estadoReleRef, (snapshot) => {
      const snap = snapshot.val();
      if(snapshot.exists()) {
        setEstadoRele(snap);
        console.log(snap);

        return;
      };
      
      
    });

  }, []);




  return (<>
    <div className={styles.nav}>
      <div>
        <h2>SAA - Familiar</h2>
      </div>
      <h2>DASHBOARD</h2>
    </div>
    <div className={styles.body}>
      <div className={styles.info}>

        <div className={styles.nivelagua}>

          <NivelAgua />
        </div>
        <div className={styles.grafico2}>


          {estadoRele ? <button className={styles.botaoDesliga} onClick={() => setEstadoRele(!estadoRele)}>DESLIGAR A rele</button>
            : <button className={styles.botaoLiga} onClick={() => setEstadoRele(!estadoRele)}>LIGAR A BOMBA</button>
          }
          
          <div className={styles.nivelturbidez}>
            {estadoRele ? <NivelTurbidez /> : <h2>A bomba esta desligada</h2>}
          </div>

        </div>
      </div>
    </div>


  </>
  )
}

import NivelAgua from "./NivelAgua"
import NivelTurbidez from "./NivelTurbidez"
import styles from '../styles/Dashboard.module.css'
import { useEffect, useState } from "react"
import { getDatabase, ref, onValue, update } from "firebase/database";


export default function Dashboard() {

  const [estadoBomba, setEstadoBomba] = useState(false);

  useEffect(() => {
    const estadoBombaRef = ref(getDatabase());
    update(estadoBombaRef, { estado_bomba: estadoBomba })
    console.log("TESTE")

  }, [estadoBomba]);

  useEffect(() => {
    const estadoBombaRef = ref(getDatabase(), '/estado_bomba/');
    onValue(estadoBombaRef, (snapshot) => {
      const snap = snapshot.val();
      if(snapshot.exists()) {
        setEstadoBomba(snap);
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


          {estadoBomba ? <button className={styles.botaoDesliga} onClick={() => setEstadoBomba(!estadoBomba)}>DESLIGAR A BOMBA</button>
            : <button className={styles.botaoLiga} onClick={() => setEstadoBomba(!estadoBomba)}>LIGAR A BOMBA</button>
          }
          
          <div className={styles.nivelturbidez}>
            {estadoBomba ? <NivelTurbidez /> : <h2>A bomba esta desligada</h2>}
          </div>

        </div>
      </div>
    </div>


  </>
  )
}

import { dirname, join } from 'path';
import { fileURLToPath } from 'url';

// Esta linha obtém o caminho completo para o arquivo atual
const filename = fileURLToPath(import.meta.url);

// Obtém o diretório do arquivo atual
const pathName = dirname(filename);

export default function configureEndpoints(app, client, pool) {
  app.post('/switch', async (req, res) => {
    const { power } = req.body;
    if (typeof power !== 'boolean') {
      return res.status(400).json({ error: 'Value must be a boolean' });
    }

    await client.publish('switch', JSON.stringify({ power }));
    return res.json({ status: 'Success', message: 'Power command sent' });
  });

  app.get('/state', async (req, res) => {
    const {rows} = await pool.query(`
        SELECT * FROM POWER_EVENTS
        ORDER BY CREATED_AT DESC
        LIMIT 1
      `)
    console.log(rows)
    return res.json(rows[0])
  })

  app.get('/graph', async (req, res) => {
    const { rows } = await pool.query(`
      SELECT 
      DATE(inserted_at) AS date,
      AVG(value) AS avarage
      FROM 
      data
      WHERE 
      inserted_at >= CURRENT_DATE - INTERVAL '7 days'
      GROUP BY 
      DATE(inserted_at)
      ORDER BY 
      DATE(inserted_at) DESC;
      `)
    return res.send(rows)
  })

  app.get('/', (req, res) => {
    return res.sendFile(join(pathName, 'index.html'));
  });

}

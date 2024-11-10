export default function configureMqttEvents(client, pool) {
  client.on('connect', () => {
    console.log(`Connected to MQTT broker.`);
    client.subscribe('data', (err) => {
      if (!err) {
        console.log('Subscribed to topic "data"');
      }
    });
    client.subscribe('power', (err) => {
      if (!err) {
        console.log('Subscribed to topic "data"');
      }
    });
  });

  client.on('message', async (topic, message) => {
    if (topic === 'data') {
      await dataEvent(message)
    }
    if (topic === 'power') {
      await powerEvent(message)
    }
  });

  async function powerEvent(message) {
    const { power } = JSON.parse(message.toString())
    const timestamp = new Date()
    await pool.query(
      'INSERT INTO POWER_EVENTS (power, created_at) VALUES ($1, $2)',
      [power , timestamp]
    )
  }

  async function dataEvent(message) {
    try {
      const data = JSON.parse(message.toString());
      const { value } = data;
      const timestamp = new Date();

      await pool.query(
        'INSERT INTO Data (value, inserted_at) VALUES ($1, $2)',
        [value, timestamp]
      );
      console.log(`Valor ${value} inserido no banco de dados.`);
    } catch (error) {
      console.error('Error processing MQTT message:', error);
    }
  }
}

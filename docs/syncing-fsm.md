# Global Syncing

## FSM

Starts *Empty*

Moves to *Running* as soon as any buffer has audio and is playing
  calculates phrase sync points (1/8s or 1/4s or 2* or 4* length ???) from length of first buffer recorded
  tracks timing and sends phrase sync actions to buffer state machines at the right times
  Phrase Sync actions need to include current frame samples offset

When a buffer gets stopped, if all buffers are stopped then move to *Stopped*

When a buffer gets cleared, if all other buffers are empty then move to *Empty*

## Sync Points

Once *Running*, track sample count against calculated sync points. Whenever sample count passes a sync point, send **Phrase Sync** action to all buffers.

Everything synced to length of first buffer.


struct segdesc {
  uint lim_15_0 : 16;  // Low bits of segment limit
  uint base_15_0 : 16; // Low bits of segment base address
  uint base_23_16 : 8; // Middle bits of segment base address
  uint type : 4;       // Segment type (see STS_ constants)
  uint s : 1;          // 0 = system, 1 = application
  uint dpl : 2;        // Descriptor Privilege Level
  uint p : 1;          // Present
  uint lim_19_16 : 4;  // High bits of segment limit
  uint avl : 1;        // Unused (available for software use)
  uint rsv1 : 1;       // Reserved
  uint db : 1;         // 0 = 16-bit segment, 1 = 32-bit segment
  uint g : 1;          // Granularity: limit scaled by 4K when set
  uint base_31_24 : 8; // High bits of segment base address
};



#define SEG(type, base, lim, dpl) 


(struct segdesc)    
{ ((lim) >> 12) & 0xffff,
	(uint)(base) & 0xffff,      
  ((uint)(base) >> 16) & 0xff, 
  type, 
  1, 
  dpl, 
  1,       
  (uint)(lim) >> 28, 
  0, 
  0, 
  1,
   1, (uint)(base) >> 24 
}







#define SEG16(type, base, lim, dpl) (struct segdesc)  \
{ (lim) & 0xffff, (uint)(base) & 0xffff,              \
  ((uint)(base) >> 16) & 0xff, type, 1, dpl, 1,       \
(uint)(lim) >> 16, 0, 0, 1, 0, (uint)(base) >> 24 }




//////////////////////////////////////////////////////////////////////////////////////v

// For example: 

#define GETVALID(mementrystruct)

(mementruct >> 31) & 0x1
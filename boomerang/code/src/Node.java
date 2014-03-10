import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.security.SecureRandom;

public class Node
{
	// Behavior parameters
	public Boomerang boom;
	public int id;
	public SecureRandom rng;
	public Location loc;
	
	// Statistics
	public long startTime;
	public long endTime;
	public long liveTime;
	public int numRetries;
	public int numForwarded;
	public int numChaffEncodings;
	public int numTxEncodings;
	
	// Delay times for forwarding and generating
	public long forwardWait;
	public boolean forward;
	public long coverWait;
	public boolean coverSendStart;
	public long txWait;
	public boolean txSendStart;
	public boolean resend;
	
	// State
	public HashSet<Message> mixBucket;
	public HashSet<Message> lastSent;
	public long txTimeoutWait;
	public HashSet<Message> messages;
	
	// Buffer things
	public ArrayList<Message> msgQueue;
	
	// Address book data structures
	public AddressBook addressBook;
	public HashSet<Node> validNodeStage;
	public boolean rebuildingAddressBook;
	public int rebuildWait;
	
	// Local message identifier
	public int coverMsgIndex = 0;
	public int txMsgIndex = 0;
	
	// For uniquely identifying nodes
	public static int globalNodeId;
	
	public Node(Location loc, Boomerang boomerang, long seed)
	{
		this.id = globalNodeId++;
		this.boom = boomerang;
		this.loc = loc;
		this.rng = new SecureRandom();
		this.msgQueue = new ArrayList<Message>();
		this.mixBucket = new HashSet<Message>();
		this.lastSent = new HashSet<Message>();
		
		this.numRetries = 0;
		this.numForwarded = 0;
		this.numChaffEncodings = 0;
		this.numTxEncodings = 0;
		
		this.forwardWait = 0L;
		this.forward = false;
		this.coverWait = 0L;
		this.coverSendStart = true;
		this.txWait = 0L;
		this.txSendStart = true;
		this.resend = false;
		
		this.addressBook = new AddressBook();
		this.validNodeStage = new HashSet<Node>(); 
		this.rebuildingAddressBook = false;
		this.rebuildWait = 0;
	}
	
	public void doEvent()
	{
		// Handle address book updates
		addressBook.update();
		
		// Handle address book refresh events
		if (!rebuildingAddressBook && (addressBook.getNumberOfInvalidNodes() < boom.config.validNodeTransmitReq && validNodeStage.size() == 0))
		{
			// Reset validation nodes
			Util.disp(this + " ADDRESS BOOK REFRESH");
			validNodeStage.clear();
			validNodeStage = boom.randomNodeSubset(boom.config.addressBookSize);
			rebuildingAddressBook = true;
			
			// Set sleep time to be proportional to 4 * max distance (for VERSION, VERACK, GETADDR, ADDR message traversal) 
			int max = 0;
			for (Node staged : validNodeStage)
			{
				double distance = this.loc.distanceTo(staged.loc);
				int tmp = (int)((int)distance / 10e8);
				if (max < tmp)
				{
					max = tmp;
				}
			}
			rebuildWait = 4 * (max + 1);
		}
		else if (rebuildingAddressBook) // sleep period while the address book is being updated
		{
			// Count down while we rebuild the address book
			rebuildWait--;
			if (rebuildWait <= 0)
			{
				rebuildingAddressBook = false;
				addressBook.updateValidNodes(validNodeStage);
			}
		}
		else
		{
			// Handle one message in each time interval
			if (msgQueue.size() > 0)
			{
				Message m = msgQueue.get(0);
				msgQueue.remove(0);
				mixBucket.add(m);
			}
			if (mixBucket.size() >= boom.config.buffSize)
			{
				forward = true;
			}
			
			// Handle forwarding
			if (forward)
			{
				for (Message m : mixBucket)
				{
					numForwarded++;
					boom.numMessages++;
					m.sendTime.add(Clock.time);
					m.transmitMessage();
					Util.disp(this.toString() + " FORWARD " + m.toString() + " TO " + m.hops.get(0));
				}
				mixBucket.clear();
			}
			
			// Handle cover generation
			if (coverWait > 0)
			{
				coverWait--;
			} 
			else if (coverSendStart)
			{
				coverSendStart = false;
				double sleep = rng.nextDouble() * boom.config.chaffGenRate;
				sleep = sleep < 0 ? (sleep * -1) % boom.config.chaffGenRate : sleep;
				coverWait = (long)sleep;
				coverSendStart = false;
			}
			else // generate cover
			{
				if (addressBook.getNumberOfValidNodes() > 0)
				{
					// Build the m circuits of length n each
					ArrayList<Message> messages = new ArrayList<Message>();
					for (int m = 0; m < boom.config.circuitWidth; m++)
					{
						ArrayList<Node> circuit = new ArrayList<Node>();
						HashSet<Integer> seen = new HashSet<Integer>();
						seen.add(id);
						for (int n = 0; n < boom.config.circuitDepth - 1; n++)
						{
							// Pick a new node not already in this circuit
							int nIndex = rng.nextInt(addressBook.getNumberOfValidNodes());
							while (seen.contains(nIndex) == true && addressBook.isValid(nIndex))
							{
								nIndex = rng.nextInt(addressBook.getNumberOfValidNodes());
							}
							
							Node node = addressBook.nodes.get(nIndex);
							circuit.add(node);
							seen.add(nIndex);
							numChaffEncodings++;
						}
						
						// New message to send
						Message newMsg = new Message("CHAFF-" + id + "-" + coverMsgIndex++, boom, MessageType.COVER);
						circuit.add(this);
						newMsg.setHops(this, circuit);
						messages.add(newMsg);					
						boom.addMessage(newMsg);
					}
					
					// Blast out each message at the same time
					for (Message m : messages)
					{
						boom.numMessages++;
						boom.numChaffGenerated++;
						boom.startedChaff.add(m);
						m.hops.add(0, this);
						m.sendTime.add(Clock.time);
						m.transmitMessage();
						Util.disp(this.toString() + " CHAFF GEN " + m.toString());
					}
					
					coverSendStart = true;
				}
			}
			
			// Handle TX generation
			if (txWait > 0)
			{
				txWait--;
			} 
			else if (txSendStart)
			{
				txSendStart = false;
				double sleep = rng.nextDouble() * boom.config.txGenRate;
				sleep = sleep < 0 ? (sleep * -1) % boom.config.txGenRate : sleep;
				txWait = (long)sleep;
				Util.disp(this.toString() + " TX SLEEP " + txWait);
			}
			else if (txTimeoutWait > 0)
			{	
				for (Message m : lastSent)
				{
					// So long as one instance was broadcasted, move on to the next message 
					if (m.broadcasted)
					{
						txTimeoutWait = 0;
						txSendStart = true;
					}
				}
				
				if (!txSendStart)
				{
					txTimeoutWait--;
					if (txSendStart == false && txTimeoutWait == 0)
					{
						resend = true;
					}
				}
			}
			else if (resend)
			{
				// Blast out each message at the same time
				resend = false;
				numRetries++;
				messages = new HashSet<Message>();
				if (addressBook.getNumberOfValidNodes() > 0)
				{
					for (int m = 0; m < boom.config.circuitWidth; m++)
					{
						ArrayList<Node> circuit = new ArrayList<Node>();
						HashSet<Integer> seen = new HashSet<Integer>();
						for (int n = 0; n < boom.config.circuitDepth; n++)
						{
							// Pick a new node not already in this circuit
							int nIndex = rng.nextInt(addressBook.getNumberOfValidNodes());
							while (seen.contains(nIndex) == true && addressBook.isValid(nIndex))
							{
								nIndex = rng.nextInt(addressBook.getNumberOfValidNodes());
							}
							
							Node node = addressBook.nodes.get(nIndex);
							circuit.add(node);
							seen.add(nIndex);
							numTxEncodings++;
							txMsgIndex++;
						}
						
						// New message to send
						Message newMsg = new Message("TX-" + id + "-" + txMsgIndex, boom, MessageType.TX);
						newMsg.setHops(this, circuit);
						messages.add(newMsg);
						boom.addMessage(newMsg);
					}
					
					// Blast out each message at the same time
					for (Message m : messages)
					{
						boom.numMessages++;
						boom.numTxGenerated++;
						boom.startedTx.add(m);
						m.sendTime.add(Clock.time);
						m.transmitMessage();
						Util.disp(this.toString() + " TX RESEND " + m.toString());
					}
					
					// Set timeout wait 
					lastSent = messages;
					txTimeoutWait = boom.config.retryLimit;
				}
			}
			else // generate FRESH TX
			{
				if (addressBook.getNumberOfValidNodes() > 0)
				{
					resend = false;
					messages = new HashSet<Message>();
					for (int m = 0; m < boom.config.circuitWidth; m++)
					{
						ArrayList<Node> circuit = new ArrayList<Node>();
						HashSet<Integer> seen = new HashSet<Integer>();
						for (int n = 0; n < boom.config.circuitDepth; n++)
						{
							// Pick a new node not already in this circuit
							int nIndex = rng.nextInt(addressBook.getNumberOfValidNodes());
							while (seen.contains(nIndex) == true && addressBook.isValid(nIndex))
							{
								nIndex = rng.nextInt(addressBook.getNumberOfValidNodes());
							}
							
							Node node = addressBook.nodes.get(nIndex);
							circuit.add(node);
							seen.add(nIndex);
							numTxEncodings++;
						}
						
						// New message to send
						Message newMsg = new Message("TX-" + id + "-" + txMsgIndex, boom, MessageType.TX);
						newMsg.setHops(this, circuit);
						messages.add(newMsg);
						boom.addMessage(newMsg);
					}
					
					// Blast out each message at the same time
					for (Message m : messages)
					{
						boom.numMessages++;
						boom.numTxGenerated++;
						boom.startedTx.add(m);
						m.sendTime.add(Clock.time);
						m.transmitMessage();
						Util.disp(this.toString() + " TX GEN " + m.toString());
					}
					
					// Set timeout wait 
					lastSent = messages;
					txTimeoutWait = boom.config.retryLimit;
				}
			}
		}
	}
	
	public void acceptMessage(Message m) 
	{
		if (m.hops.size() > 2)
		{
			Util.disp(this.toString() + " ACCEPTED " + m.toString());
			m.arriveTime.add(Clock.time);
			msgQueue.add(m);
		}
		else
		{	
			// Reached the end of the circuit, broadcast the transaction here
			if (m.type == MessageType.TX)
			{
				m.markAsBroadcasted();
				Util.disp(m.toString() + " MSG BROADCASTED");
			}
			else
			{
				Util.disp(m.toString() + " EOL");
			}
		}
	}
	
	@Override
	public String toString()
	{
		return "N-" + id;
	}
}

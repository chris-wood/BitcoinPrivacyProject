
public class Clock
{
	public static long time;
	
	public Clock()
	{
		time = 0L;
	}
	
	public void tick()
	{
		time++;
		System.err.println("Time: " + time);
	}
}

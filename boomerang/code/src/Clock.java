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
		Util.error("Time: " + time);
	}
}

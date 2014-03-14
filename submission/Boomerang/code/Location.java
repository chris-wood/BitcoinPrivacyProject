
public class Location
{
	public int x;
	public int y;
	
	public Location(int xx, int yy)
	{
		this.x = xx;
		this.y = yy;
	}
	
	public double distanceTo(Location other)
	{
		// D = sqrt(dy^2 + dx^2)
		return Math.sqrt(Math.pow(Math.abs((double)other.x - x), 2) + Math.pow(Math.abs((double)other.y - y), 2));
	}
}

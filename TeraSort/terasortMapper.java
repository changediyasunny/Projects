
import java.io.IOException;
import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Job;
import org.apache.hadoop.mapreduce.Mapper;
import org.apache.hadoop.mapreduce.Reducer;
import org.apache.hadoop.mapreduce.lib.input.FileInputFormat;
import org.apache.hadoop.mapreduce.lib.output.FileOutputFormat;
import org.apache.hadoop.mapreduce.Counter;
import org.apache.hadoop.util.StringUtils;


public class terasortMapper extends Mapper<Object, Text, Text, Text> 
{

	
	@Override
	public void map(Object key, Text value, Context con) throws IOException, InterruptedException 
	{
		
		//Text value containe the complete line...
		//System.out.println(value.toString());
				
		String key_part = value.toString().substring(0,10);
		//System.out.println(key_part); //pefectly prints key
		
		String value_part = value.toString().substring(10, value.toString().length());
		//System.out.println(value_part);
		
		con.write(new Text(key_part), new Text(value_part));
	}
}


public class Halo {
	private greeting;

	public Halo(String greeting) {
		this.greeting = greeting;
	}

	public void sayGreeting() {
		System.out.println(String.Format("%s", this.greeting));
	}

	public void changeGreeting(String newGreeting) {
		this.greeting = newGreeting;
	}

	public static void main(String args[]) {
		Halo hello = new Halo("Grettings, this is my humble java code but this is a 'C' only development, so this code shall never run which is 'Sad But True' Metalica would say!?");
		hello.sayGreeting();
	}
}
